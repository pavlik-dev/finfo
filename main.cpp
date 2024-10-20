#define TABS "  "

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <dirent.h>
#include <array>
// #include <sys/types.h>
#include <sys/stat.h>
// #include <fcntl.h>

#include "mime.cpp"
#include "Field.cpp"
#include "Extension.cpp"
using namespace std;

array<int, 4> get_files_in_directory(const string& path)
{
  DIR* dp = opendir(path.c_str());
  // int normal, hidden;
  array<int, 2> normal = {0, 0};
  array<int, 2> hidden = {0, 0};
  if (dp != nullptr) {
    struct dirent* ep;
    while((ep = readdir(dp))) {
      string name(ep->d_name);
      if (name != "." && name != "..") {
        if (name[0] == '.') {
          hidden[(ep->d_type == DT_DIR) ? 1 : 0]++;
        }else {
          normal[(ep->d_type == DT_DIR) ? 1 : 0]++;
        }
      }
    }

    (void)closedir(dp);
  }
  else
    perror("Couldn't open the directory");
  array<int, 4> result;
  result[0] = normal[0];
  result[1] = normal[1];
  result[2] = hidden[0];
  result[3] = hidden[1];
  return result;
}

string basename(string path, bool is_dir=false) {
  return (path.substr(path.find_last_of("/") + 1))+((is_dir) ? "/" : "");
}

string print_permissions(struct stat _stat) {
  mode_t mode = _stat.st_mode;
  bool is_dir = _stat.st_mode & S_IFDIR;
  string user;
  user += (mode & S_IRUSR) ? "r" : "-";
  user += (mode & S_IWUSR) ? "w" : "-",
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

string get_file_type(struct stat _stat) {
  switch (_stat.st_mode & S_IFMT) {
    case S_IFBLK:  return("block device");     break;
    case S_IFCHR:  return("character device"); break;
    case S_IFDIR:  return("directory/folder"); break;
    case S_IFIFO:  return("FIFO/pipe");        break;
    case S_IFLNK:  return("symlink");          break;
    case S_IFREG:  return("regular file");     break;
    case S_IFSOCK: return("socket");           break;
    default:       return("unknown");          break;
   }
}

string readable_fs(const long int size /*in bytes*/,
      const double divide_by = 1024,
      const string& suffix = "B")
{
  if(size < 1024) {
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

int print_usage() {
  cerr << "Usage:" << endl;
  cerr << TABS << argv[0] << " [args] <files>" << endl;
  return 1;
}

int main(int argc, char *argv[])
{
  if (argc < 2) {
    return print_usage();
  }
  vector<string> files;
  vector<string> args;

  bool files_started = false;
  for (int i = 1; i < argc; ++i) {
    if (argv[i][0] == '-' && !files_started) {
      if (argv[i] != "--")
        args.push_back(string(argv[i]));
      else
        files_started = true;
    } else {
      files.push_back(string(argv[i]));
    }
  }

  if (files.size() < 1) {
    return print_usage();
  }

  bool first = true;
  for (const string& file : files) {
    char* temp = (char*)malloc(4097);
    temp[4096] = '\0';
    if (realpath(file.c_str(), temp) == nullptr) {
      perror("realpath");
      return 1;
    }
    string abspath(temp);
    free(temp);

    struct stat file_stat;
    if (stat(file.c_str(), &file_stat) != 0) {
      string errdesc = "stat error at file "+file;
      perror(errdesc.c_str());
      continue;
    }
    bool is_dir = file_stat.st_mode & S_IFDIR;

    Field type = quick_field("Type", get_file_type(file_stat));
    Field mime = quick_field("MIME", get_mime(file));
    Field size = quick_field("Size", readable_fs((long)file_stat.st_size));
    Field dircont = quick_field("Contents", "");
    if (is_dir) {
      int buffer_size = 128;
      auto insides = get_files_in_directory(file);
      char *buffer = (char*)malloc(buffer_size);
      if (buffer == NULL) {
          // Check if malloc failed
          fprintf(stderr, "Memory allocation failed\n");
          return 1;
      }
      snprintf(buffer, buffer_size, "%i files (+%i hidden)", insides[0], insides[1]);
      string _str_files = buffer;
      free(buffer);
      char temp[4097] = {'\0'};

      buffer = (char*)malloc(buffer_size);
      if (buffer == NULL) {
          // Check if malloc failed
          fprintf(stderr, "Memory allocation failed\n");
          return 1;
      }
      snprintf(buffer, buffer_size, "%i folders (+%i hidden)", insides[2], insides[3]);
      string _str_dirs = buffer;
      Field files = quick_field(_str_files, "", false);
      Field dirs = quick_field(_str_dirs, "", false);
      dircont.subfields.push_back(files);
      dircont.subfields.push_back(dirs);
      free(buffer);
    }

    char buffer[8];
    snprintf(buffer, sizeof(buffer), " (%3o)", file_stat.st_mode & 0777);
    string _perms(buffer);
    Field perms = quick_field("Permissions", print_permissions(file_stat)+_perms);

    Field start;
    start.name = "\x1b[1m"+basename(abspath, is_dir)+"\x1b[0m";
    start.subfields.push_back(type);
    start.subfields.push_back(mime);
    start.subfields.push_back(size);
    start.subfields.push_back(perms);
    if (is_dir) {
      start.subfields.push_back(dircont);
    }

    print_field(start);
    if (first)
      cout << endl;
    first = false;
  }
  return 0;
}
