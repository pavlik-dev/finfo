#define TABS "  "
#define COLOR 0

#include <stdio.h>

#include <array>
#include <ctime>
#include <dirent.h>
#include <fcntl.h>
#include <iostream>
#include <memory>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <sstream>

#ifndef _WIN32
#include <mntent.h>
#else
#include <windows.h>
#include <winuser.h>
#endif

using namespace std;

#include "Field.cpp"
#include "File.cpp"
#include "mime.cpp"

// Maybe move this to string_ops.cpp?
string trim(const string &str, const string &whitespace = " \t\n")
{
  const auto strBegin = str.find_first_not_of(whitespace);
  if (strBegin == string::npos)
    return ""; // no content

  const auto strEnd = str.find_last_not_of(whitespace);
  const auto strRange = strEnd - strBegin + 1;

  return str.substr(strBegin, strRange);
}

inline bool file_exists(const string &name) { return __file_exists(name); }

struct DirContents
{
  int normal_files = 0, hidden_files = 0;
  int normal_dirs = 0, hidden_dirs = 0;
};

DirContents get_files_in_directory(const string &path)
{
  DirContents result;
#ifndef _WIN32
  if (DIR *dp = opendir(path.c_str()))
  {
    while (auto ep = readdir(dp))
    {
      string name(ep->d_name);
      if (name == "." || name == "..")
        continue;
      if (ep->d_type == DT_DIR)
        name[0] == '.' ? ++result.hidden_dirs : ++result.normal_dirs;
      else
        name[0] == '.' ? ++result.hidden_files : ++result.normal_files;
    }
    closedir(dp);
  }
  else
  {
    perror("Couldn't open the directory");
  }
#else
  WIN32_FIND_DATAA findFileData;
  HANDLE hFind = FindFirstFileA((path + "\\*").c_str(), &findFileData);
  if (hFind != INVALID_HANDLE_VALUE)
  {
    do
    {
      string name(findFileData.cFileName);
      if (name == "." || name == "..")
        continue;
      bool isDir = (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
      bool isHidden = (findFileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0;
      if (isDir)
        isHidden ? ++result.hidden_dirs : ++result.normal_dirs;
      else
        isHidden ? ++result.hidden_files : ++result.normal_files;
    } while (FindNextFileA(hFind, &findFileData));
    FindClose(hFind);
  }
  else
  {
    perror("Couldn't open the directory");
  }
#endif
  return result;
}

string print_permissions(struct STAT _stat)
{
  mode_t mode = _stat.st_mode;
#ifndef _WIN32
  bool is_dir = _stat.st_mode & S_IFDIR;
#else
  bool is_dir = _stat.st_mode & _S_IFDIR;
#endif
  string user;
  user += (mode & S_IRUSR) ? "r" : "-";
  user += (mode & S_IWUSR) ? "w" : "-";
  user += (mode & S_IXUSR) ? "x" : "-";

  string group;
  group += (mode & S_IRGRP) ? "r" : "-";
  group += (mode & S_IWGRP) ? "w" : "-";
  group += (mode & S_IXGRP) ? "x" : "-";

  string other;
  other += (mode & S_IROTH) ? "r" : "-";
  other += (mode & S_IWOTH) ? "w" : "-";
  other += (mode & S_IXOTH) ? "x" : "-";

  return (is_dir ? "d" : "-") + user + group + other;
}

string get_file_type(struct STAT _stat, const string filename)
{
  switch (_stat.st_mode & S_IFMT)
  {
  case S_IFBLK:
    return ("block device");
  case S_IFCHR:
    return ("character device");
  case S_IFDIR:
    return ("directory/folder");
  case S_IFIFO:
    return ("FIFO/pipe");
#ifndef _WIN32
  case S_IFLNK:
    return ("symlink");
  case S_IFSOCK:
    return ("socket");
#endif
  case S_IFREG:
    return get_mime(filename);

  default:
    return ("unknown");
  }
}

string readable_fs(const long int size /*in bytes*/,
                   const double divide_by = 1024, const string &suffix = "B")
{
  if (size < 1024)
  {
    char buffer[128];
    snprintf(buffer, 128, "%i B", static_cast<int>(size));
    string result(buffer);
    return buffer;
  }
  auto result = static_cast<double>(size);
  int i = 0;
  const string units[] = {"", "K", "M", "G", "T", "P", "E", "Z", "Y"};
  while (result > divide_by)
  {
    result /= divide_by;
    i++;
  }
  char buffer[128];
  snprintf(buffer, 128, "%.*f %s%s", i, result, units[i].c_str(), suffix.c_str());
  string str_result(buffer);
  return str_result;
}

int print_usage(char *argv[])
{
  cerr << "Usage:" << endl;
  cerr << TABS << argv[0] << " <files>" << endl;
  return 1;
}

string exec(string cmd)
{
  shared_ptr<FILE> pipe(popen(cmd.c_str(), "r"), pclose);
  if (!pipe)
    return "ERROR";
  char buffer[128];
  string result = "";
  while (!feof(pipe.get()))
  {
    if (fgets(buffer, 128, pipe.get()) != NULL)
      result += buffer;
  }
  return result;
}

// https://stackoverflow.com/a/4643526
string escape_quotes(string str)
{
  size_t index = 0;
  while (true)
  {
    /* Locate the substring to replace. */
    index = str.find("\"", index);
    if (index == std::string::npos)
      break;

    /* Make the replacement. */
    str.replace(index, 1, "\\\"");

    /* Advance index forward so the next iteration doesn't pick it up as well.
     */
    index += 2;
  }
  return str;
}

inline string basename(string path, bool is_dir = false)
{
  return (path.substr(path.find_last_of(DELIM) + 1)) + ((is_dir) ? DELIM : "");
}

int main(int argc, char *argv[])
{
  if (argc < 2)
    return print_usage(argv);

  vector<string> files;
  for (int i = 1; i < argc; ++i)
    files.push_back(string(argv[i]));

  int counter = 0;
  for (const string &file : files)
  {
    counter++;
    File file_obj;
    bool success = false;
    vector<Field> fields;
    string abspath = file;
    bool is_dir = false;
    try
    {
      file_obj = File(file);
      success = true;
    }
    catch (const NotFoundException &e)
    {
      fields.push_back(Field("Error", "Not found"));
    }
    catch (const StatFailedException &e)
    {
      fields.push_back(Field("Error", "Stat failed"));
    }
    if (success)
    {
      struct STAT file_stat = file_obj.file_stat;
      abspath = file_obj.abs_path;

      string file_type = get_file_type(file_stat, abspath);

      is_dir = (file_stat.st_mode & S_IFMT) == S_IFDIR;
      bool is_file = (file_stat.st_mode & S_IFMT) == S_IFREG;
      bool is_device = (file_stat.st_mode & S_IFMT) == S_IFBLK ||
                       (file_stat.st_mode & S_IFMT) == S_IFCHR;

      if (!is_file)
        fields.push_back(Field("Type", file_type));

      if (is_dir)
      {
        Field dircont("Contents", "");
        auto insides = get_files_in_directory(file);

        string filesText = to_string(insides.normal_files) + " (+" +
                           to_string(insides.hidden_files) + " hidden)";
        string dirsText = to_string(insides.normal_dirs) + " (+" +
                          to_string(insides.hidden_dirs) + " hidden)";

        Field files("Files", filesText);
        Field dirs("Directories", dirsText);

        dircont.add_field(files);
        dircont.add_field(dirs);
        fields.push_back(dircont);
      }
      else if (is_file)
      {
        string safe_abs = escape_quotes(string(abspath));

#if !defined(NO_MIME) && !defined(_WIN32)
        string output = exec("file --mime-type \"" + safe_abs + "\"");
        fields.push_back(
            Field("Type", output.size() > 0
                              ? trim(output.substr(abspath.size() + 2))
                              : "failed"));

        output = exec("file --mime-encoding \"" + safe_abs + "\"");
        fields.push_back(
            Field("Encoding", output.size() > 0
                                  ? trim(output.substr(abspath.size() + 2))
                                  : "failed"));
#endif
        fields.push_back(Field("Size", readable_fs((long)file_stat.st_size)));
      }
      else if (is_device)
      {
        // Terminal check
        int fd = open(abspath.c_str(), O_RDONLY);
        fields.push_back(Field("Is a terminal", fd < 0 && isatty(fd) ? "yes" : "no"));
        close(fd);

        // Mount point check
#ifndef _WIN32
        struct mntent *ent;
        FILE *aFile;

        aFile = setmntent("/proc/mounts", "r");
        if (aFile == NULL)
        {
          perror("setmntent");
          fields.push_back(Field("Mount point: ", "N/A"));
        }
        else
        {
          string mount_points = "";
          size_t total = 0;
          while (NULL != (ent = getmntent(aFile)))
            if (ent->mnt_fsname == abspath)
            {
              if (total > 32)
              {
                mount_points += ", ...";
                break;
              } // Too much mount points!
              total += 1;
              mount_points += ((mount_points != "") ? ", " : "") + string(ent->mnt_dir);
            }

          if (mount_points != "") fields.push_back(Field("Mount point(s)", mount_points));
          endmntent(aFile);
        }
#endif
      }

      char buffer[8];
      snprintf(buffer, sizeof(buffer), " (%3o)", file_stat.st_mode & 0777);
      string _perms(buffer);
      fields.push_back(
          Field("Permissions", print_permissions(file_stat) + _perms));

      struct tm *timeinfo;
      Field dates("Last", "");
#ifdef __APPLE__ // macOS
      timeinfo = localtime(&file_stat.st_birthtime);
      dates.add_field(Field("Creation date", trim(string(asctime(timeinfo)))));
#endif
      timeinfo = localtime(&file_stat.st_atime);
      dates.add_field(Field("opened", trim(string(asctime(timeinfo)))));

      timeinfo = localtime(&file_stat.st_mtime);
      dates.add_field(Field("modified", trim(string(asctime(timeinfo)))));

      fields.push_back(dates);
    }
    Field start((COLOR ? "\x1b[1m" : "") + basename(abspath, is_dir) +
                    (COLOR ? "\x1b[0m" : ""),
                "");

    for (size_t i = 0; i < fields.size(); ++i)
      start.add_field(fields[i]);

    string output = start.print();
    cout << output;
  }
  return 0;
}
