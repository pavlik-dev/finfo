#define TABS "  "
#define COLOR 0

#include <stdio.h>

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
//#include "mime.cpp"

string itostring(const int a) {
  char *intStr = (char*)malloc(32);
  itoa(a, intStr, 10);
  string str = string(intStr);
  free(intStr);
  return str;
}

// Maybe move this to string_ops.cpp?
string trim(const string &str, const string &whitespace = " \t\n")
{
  const size_t strBegin = str.find_first_not_of(whitespace);
  if (strBegin == string::npos)
    return ""; // no content

  const size_t strEnd = str.find_last_not_of(whitespace);
  const size_t strRange = strEnd - strBegin + 1;

  return str.substr(strBegin, strRange);
}

//inline bool file_exists(const string &name) { return __file_exists(name); }

struct DirContents
{
  int normal_files, hidden_files;
  int normal_dirs,  hidden_dirs;
};

struct DirContents get_files_in_directory(const string &path)
{
  struct DirContents result;
  result.hidden_dirs = 0;
  result.hidden_files = 0;
  result.normal_dirs = 0;
  result.normal_files = 0;
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
  group += (mode & (0400 >> 3)) ? "r" : "-";
  group += (mode & (0200 >> 3)) ? "w" : "-";
  group += (mode & (0100 >> 3)) ? "x" : "-";

  string other;
  other += (mode & (0400 >> 6)) ? "r" : "-";
  other += (mode & (0200 >> 6)) ? "w" : "-";
  other += (mode & (0100 >> 6)) ? "x" : "-";
#ifdef KFJDKFSDKLFJL
  string group = "---";
  string other = "---";
#endif
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
  case S_IFREG:
    return ("regular file");

  default:
    return ("unknown");
  }
}

string readable_fs(const long int size /*in bytes*/,
                   const double divide_by = 1024, const string &suffix = "B")
{
  if (size < 1024)
  {
    return itostring(size)+" B";
  }
  double result = static_cast<double>(size);
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

#ifndef _WIN32
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
#endif

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

  bool detailed = false;

  vector<string> files;
  for (int i = 1; i < argc; ++i)
    files.push_back(string(argv[i]));

  for (int counter = 0;counter<files.size();++counter)
  {
    string file = files[counter];
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
        struct DirContents insides = get_files_in_directory(file);
        
        string filesText = itostring(insides.normal_files) + " files (+" +
                           itostring(insides.hidden_files) + " hidden)";
        
        string dirsText = itostring(insides.normal_dirs) + " folders (+" +
                          itostring(insides.hidden_dirs) + " hidden)";

        Field files(filesText, "");
        Field dirs(dirsText, "");

        dircont.add_field(files);
        dircont.add_field(dirs);
        //dircont.add_field(Field("Total", itostring(total)));
        fields.push_back(dircont);
      }
      else if (is_file)
      {
        fields.push_back(Field("Size", readable_fs((long)file_stat.st_size)));
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
