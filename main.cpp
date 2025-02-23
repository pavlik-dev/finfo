/* шо ты родненькая */
#define TABS "  "
#define COLOR 0

#include <cstdio>

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

#ifndef _WIN32
#include <mntent.h>
#else
#include <windows.h>
#endif

using namespace std;

#include "Field.cpp"
#include "file.cpp"
#include "mime.cpp"

string trim(const string &str, const string &whitespace = " \t\n") {
  const auto strBegin = str.find_first_not_of(whitespace);
  if (strBegin == string::npos)
    return ""; // no content

  const auto strEnd = str.find_last_not_of(whitespace);
  const auto strRange = strEnd - strBegin + 1;

  return str.substr(strBegin, strRange);
}

string dirnameOf(const string &fname) {
  size_t pos = fname.find_last_of("\\/");
  return (string::npos == pos) ? "" : fname.substr(0, pos);
}

inline string get_extension(const string &filename) {
  return filename.substr(filename.find_last_of(".") + 1);
}

// Source: https://stackoverflow.com/a/12774387
// Yeah I stole this from SO :P
inline bool file_exists(const string &name) { return __file_exists(name); }

#ifdef _WIN32
std::array<int, 4> get_files_in_directory(const std::string &path) {
  WIN32_FIND_DATAA findFileData;
  HANDLE hFind = FindFirstFileA((path + "\\*").c_str(), &findFileData);

  std::array<int, 2> normal = {0, 0};
  std::array<int, 2> hidden = {0, 0};

  if (hFind != INVALID_HANDLE_VALUE) {
    do {
      std::string name(findFileData.cFileName);
      if (name != "." && name != "..") {
        bool isDir =
            (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
        bool isHidden =
            (findFileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0;

        if (isHidden) {
          hidden[isDir ? 1 : 0]++;
        } else {
          normal[isDir ? 1 : 0]++;
        }
      }
    } while (FindNextFileA(hFind, &findFileData));

    FindClose(hFind);
  } else {
    std::cerr << "Couldn't open the directory" << std::endl;
  }

  std::array<int, 4> result = {normal[0], normal[1], hidden[0], hidden[1]};
  return result;
}
#else
array<int, 4> get_files_in_directory(const string &path) {
  DIR *dp = opendir(path.c_str());
  // int normal, hidden;
  array<int, 2> normal = {0, 0};
  array<int, 2> hidden = {0, 0};
  if (dp != nullptr) {
    struct dirent *ep;
    while ((ep = readdir(dp))) {
      string name(ep->d_name);
      if (name != "." && name != "..") {
        if (name[0] == '.') {
          hidden[(ep->d_type == DT_DIR) ? 1 : 0]++;
        } else {
          normal[(ep->d_type == DT_DIR) ? 1 : 0]++;
        }
      }
    }

    (void)closedir(dp);
  } else
    perror("Couldn't open the directory");
  array<int, 4> result;
  result[0] = normal[0];
  result[1] = normal[1];
  result[2] = hidden[0];
  result[3] = hidden[1];
  return result;
}
#endif

#ifndef _WIN32
string print_permissions(struct STAT _stat) {
  mode_t mode = _stat.st_mode;
  bool is_dir = _stat.st_mode & S_IFDIR;
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
#else
string print_permissions(struct STAT _stat) {
  mode_t mode = _stat.st_mode;
  bool is_dir = _stat.st_mode & _S_IFDIR;
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
#endif

string get_file_type(struct STAT _stat, const string filename) {
  switch (_stat.st_mode & S_IFMT) {
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
                   const double divide_by = 1024, const string &suffix = "B") {
  if (size < 1024) {
    char buffer[128];
    sprintf(buffer, "%i B", static_cast<int>(size));
    string result(buffer);
    return buffer;
  }
  auto result = static_cast<double>(size);
  int i = 0;
  const string units[] = {"", "K", "M", "G", "T", "P", "E", "Z", "Y"};
  while (result > divide_by) {
    result /= divide_by;
    i++;
  }
  char buffer[128];
  sprintf(buffer, "%.*f %s%s", i, result, units[i].c_str(), suffix.c_str());
  string str_result(buffer);
  return str_result;
}

int print_usage(char *argv[]) {
  cerr << "Usage:" << endl;
  cerr << TABS << argv[0] << " <files>" << endl;
  return 1;
}

string exec(string cmd) {
  shared_ptr<FILE> pipe(popen(cmd.c_str(), "r"), pclose);
  if (!pipe)
    return "ERROR";
  char buffer[128];
  string result = "";
  while (!feof(pipe.get())) {
    if (fgets(buffer, 128, pipe.get()) != NULL)
      result += buffer;
  }
  return result;
}

// https://stackoverflow.com/a/4643526
string escape_quotes(string str) {
  size_t index = 0;
  while (true) {
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

int main(int argc, char *argv[]) {
  if (argc < 2)
    return print_usage(argv);

  vector<string> files;
  for (int i = 1; i < argc; ++i)
    files.push_back(string(argv[i]));

  bool first = true;
  int counter = 0;
  for (const string &file : files) {
    if (!first)
      cout << endl;
    counter++;
    File file_obj;
    bool success = false;
    vector<Field> fields;
    string abspath = file;
    bool is_dir = false;
    try {
      file_obj = File(file);
      success = true;
    } catch (const NotFoundException &e) {
      fields.push_back(Field("Error", "Not found"));
    } catch (const StatFailedException &e) {
      fields.push_back(Field("Error", "Stat failed"));
    }
    if (success) {
      struct STAT file_stat = file_obj.file_stat;
      abspath = file_obj.abs_path;

      string file_type = get_file_type(file_stat, abspath);

      is_dir = file_stat.st_mode & S_IFMT & S_IFDIR;
      bool is_file = file_stat.st_mode & S_IFMT & S_IFREG;
      bool is_device = file_stat.st_mode & S_IFMT & S_IFBLK ||
                       file_stat.st_mode & S_IFMT & S_IFCHR;

      fields.push_back(Field("Type", file_type));

      if (is_dir) {
        Field dircont("Contents", "");
        auto insides = get_files_in_directory(file);

        string filesText = to_string(insides[0]) + " files (+" +
                           to_string(insides[1]) + " hidden)";
        string dirsText = to_string(insides[2]) + " folders (+" +
                          to_string(insides[3]) + " hidden)";

        Field files(filesText, "");
        Field dirs(dirsText, "");

        dircont.add_field(files);
        dircont.add_field(dirs);
        fields.push_back(dircont);
      } else if (is_file) {
        string safe_abs = escape_quotes(string(abspath));

#if !defined(NO_MIME) && !defined(_WIN32)
        string output = exec("file --mime-encoding \"" + safe_abs + "\"");
        fields.push_back(
            Field("Encoding", output.size() > 0
                                  ? trim(output.substr(abspath.size() + 2))
                                  : "failed"));
#endif
        fields.push_back(Field("Size", readable_fs((long)file_stat.st_size)));
      } else if (is_device) {
        // Terminal check
        int fd = open(abspath.c_str(), O_RDONLY);
        if (fd < 0) {
          perror("open");
        } else {
          if (isatty(fd))
            fields.push_back(Field("Is a terminal", "yes"));
          close(fd);
        }

// Mount point check
#ifndef _WIN32
        struct mntent *ent;
        FILE *aFile;

        aFile = setmntent("/proc/mounts", "r");
        if (aFile == NULL) {
          perror("setmntent");
          fields.push_back(Field("Mount point: ", "N/A"));
        } else {
          while (NULL != (ent = getmntent(aFile)))
            if (ent->mnt_fsname == abspath)
              fields.push_back(Field("Mount point", ent->mnt_dir));
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

    start.print();
    first = false;
  }
  return 0;
}
