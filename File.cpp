#include "platform.hpp"

#include <string>
#include <vector>
#include <stdexcept>
#include <cstdio>
#include <sstream>
#include <cstdlib>
#include <unistd.h> // for access() and realpath()

#if PLATFORM == 2
// Linux-specific includes for statx
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <sys/sysmacros.h>
#ifndef NO_DIRENT
#include <dirent.h>
#endif
// Ensure STATX_ALL is defined
#ifndef STATX_ALL
#define STATX_ALL 0xFFF
#endif
#ifdef NO_STATX
#define STAT stat
#endif
#define DELIM "/"
// We'll be using statx() directly.
#elif PLATFORM == 1
#include <windows.h>
#include <direct.h>
#include <fileapi.h>
#define STAT _stat
#define DELIM "\\"
#define realpath(N, R) _fullpath((R), (N), _MAX_PATH)
#else
#include <sys/stat.h>
#define STAT stat
#define DELIM "/"
#endif

#ifndef _MAX_PATH
#define _MAX_PATH 1024
#endif

class FileException : public std::runtime_error
{
public:
  explicit FileException(const std::string &msg) : std::runtime_error(msg) {}
};

// enum FileType {
//   BLOCK_DEVICE,
//   CHAR_DEVICE,
//   DIRECTORY,
//   FIFO,
//   SYMLINK,
//   SOCKET,
//   REGULAR,
//   UNKNOWN
// };

struct DirContents
{
  unsigned int normal_files, hidden_files;
  unsigned int normal_dirs, hidden_dirs;
};

class File
{
public:
#if PLATFORM == 2 && !defined(NO_STATX)
  struct statx file_statx;
#else
  struct STAT file_stat;
#endif

  std::string abs_path;
  std::string file_name;

  dev_t st_dev;   /* ID of device containing file */
  ino_t st_ino;   /* Inode number */
  mode_t st_mode; /* File type and mode */

  unsigned int permissions;
  unsigned int file_type;

#if PLATFORM != 1
  nlink_t st_nlink;     /* Number of hard links */
  uid_t st_uid;         /* User ID of owner */
  gid_t st_gid;         /* Group ID of owner */
  dev_t st_rdev;        /* Device ID (if special file) */
  off_t st_size;        /* Total size, in bytes */
  blksize_t st_blksize; /* Block size for filesystem I/O */
  blkcnt_t st_blocks;   /* Number of 512 B blocks allocated */

  timespec btime;  /* Birth date, in most platforms falls back to last status change. */
  timespec mtime;  /* Modify date */
  timespec atime;  /* Access date */
#endif

  // Check file existence
  static bool exists(const std::string &name)
  {
#if PLATFORM == 2
    return access(name.c_str(), F_OK) == 0;
#else
    struct STAT buffer;
    return STAT(name.c_str(), &buffer) == 0;
#endif
  }

  File() {};

  // Constructor: verifies file existence, resolves absolute path, and retrieves stats.
  File(const std::string &file)
  {
    file_name = file;

    // Check for existence
    if (!File::exists(file))
      throw FileException("File not found: " + file);

    // Resolve absolute path
    std::vector<char> temp(_MAX_PATH);
    if (realpath(file.c_str(), &temp[0]) == NULL)
    {
      perror("realpath");
      throw FileException("Failed to resolve absolute path for: " + file);
    }
    abs_path = std::string(&temp[0]);

    // Retrieve file statistics
#if PLATFORM == 2 && !defined(NO_STATX)
    // Call statx: use AT_FDCWD and AT_SYMLINK_NOFOLLOW for a non-following call.
    if (statx(AT_FDCWD, abs_path.c_str(), AT_SYMLINK_NOFOLLOW, STATX_ALL, &file_statx) != 0)
    {
      perror("statx");
      throw FileException("Failed to retrieve file statistics for: " + abs_path);
    }
#else
    if (STAT(abs_path.c_str(), &file_stat) != 0)
    {
      perror(("stat error on file: " + abs_path).c_str());
      throw FileException("Failed to retrieve file statistics for: " + abs_path);
    }
#endif
#if PLATFORM == 1
    HANDLE k = CreateFileA((LPCSTR)this->abs_path, 0, 0, NULL, OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL);
#endif

/* Now those stat fields
   I noticed that a lot of fields (like st_blksize) don't match their types.
   But who cares.
*/
#if PLATFORM == 2 && !defined(NO_STATX)
    this->st_dev = makedev(file_statx.stx_dev_major, file_statx.stx_dev_minor);
    this->st_ino = file_statx.stx_ino;
    this->st_mode = file_statx.stx_mode;
    this->st_nlink = file_statx.stx_nlink;
    this->st_uid = file_statx.stx_uid;
    this->st_gid = file_statx.stx_gid;
    this->st_rdev = makedev(file_statx.stx_rdev_major, file_statx.stx_rdev_minor);
    this->st_size = file_statx.stx_size;
    this->st_blksize = file_statx.stx_blksize;
    this->st_blocks = file_statx.stx_blocks;

    this->btime.tv_sec = file_statx.stx_btime.tv_sec;
    this->btime.tv_nsec = file_statx.stx_btime.tv_nsec;

    this->mtime.tv_sec = file_statx.stx_mtime.tv_sec;
    this->mtime.tv_nsec = file_statx.stx_mtime.tv_nsec;

    this->atime.tv_sec = file_statx.stx_atime.tv_sec;
    this->atime.tv_nsec = file_statx.stx_atime.tv_nsec;
#else
    this->st_dev = file_stat.st_dev;
    this->st_ino = file_stat.st_ino;
    this->st_mode = file_stat.st_mode;

    this->btime.tv_sec = file_stat.st_ctime;
    this->btime.tv_nsec = 0;
    this->mtime.tv_sec = file_stat.st_mtime;
    this->mtime.tv_nsec = 0;
    this->atime.tv_sec = file_stat.st_atime;
    this->atime.tv_nsec = 0;

#if PLATFORM != 1
    this->st_nlink = file_stat.st_nlink;
    this->st_uid = file_stat.st_uid;
    this->st_gid = file_stat.st_gid;
    this->st_rdev = file_stat.st_rdev;
    this->st_size = file_stat.st_size;
    this->st_blksize = file_stat.st_blksize;
    this->st_blocks = file_stat.st_blocks;
#endif
#endif

/* Custom fields */
#if PLATFORM != 1
    this->file_type = this->st_mode & S_IFMT;
#else
    this->file_type = this->st_mode & _S_IFMT;
#endif
    this->permissions = this->st_mode & 0777;
  }

  // Returns the directory component of a path.
  static std::string dirname_of(const std::string &fname)
  {
    size_t pos = fname.find_last_of("\\/");
    return (pos == std::string::npos) ? "" : fname.substr(0, pos);
  }

  // Returns the base name (file name) from a path.
  static std::string basename(const std::string &path, bool is_dir = false)
  {
    std::string base = path.substr(path.find_last_of(DELIM) + 1);
    if (is_dir && !base.empty() && base[base.size() - 1] != DELIM[0])
      base.push_back(DELIM[0]);
    return base;
  }

  // Returns the extension from the filename.
  static std::string get_extension(const std::string &filename)
  {
    size_t pos = filename.find_last_of('.');
    return (pos == std::string::npos) ? "" : filename.substr(pos + 1);
  }

  // Converts file size to a human-readable std::string.
  std::string readable_fs() const
  {
    long size;
#if PLATFORM == 2 && !defined(NO_STATX)
    size = file_statx.stx_size;
#else
    size = file_stat.st_size;
#endif
    std::stringstream stream;
    if (size < 1024)
    {
      stream << size << " B";
      return stream.str();
    }

    double result = size;
    const std::string units[] = {"B", "KB", "MB", "GB", "TB", "PB", "EB"};
    int i = 0;
    while (result >= 1024 && i < 6)
    {
      result /= 1024;
      i++;
    }
    stream << result << " " << units[i];
    return stream.str();
  }

#ifndef NO_DIRENT
  DirContents get_files_in_directory() const
  {
    DirContents result;
    /* Don't move this to DirContents, please! */
    result.hidden_dirs = 0;
    result.hidden_files = 0;
    result.normal_dirs = 0;
    result.normal_files = 0;

#if PLATFORM != 1
    if (DIR *dp = opendir(abs_path.c_str()))
    {
      while (dirent *ep = readdir(dp))
      {
        std::string name(ep->d_name);
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
    HANDLE hFind = FindFirstFileA((abs_path + "\\*").c_str(), &findFileData);
    if (hFind != INVALID_HANDLE_VALUE)
    {
      do
      {
        std::string name(findFileData.cFileName);
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
#endif

  std::string get_permissions() const
  {
#if PLATFORM == 1
    bool is_dir = st_mode & _S_IFDIR;
#else
    bool is_dir = st_mode & S_IFDIR;
#endif
    std::string user;
    user += (permissions & S_IRUSR) ? "r" : "-";
    user += (permissions & S_IWUSR) ? "w" : "-";
    user += (permissions & S_IXUSR) ? "x" : "-";

    std::string group;
    group += (permissions & S_IRGRP) ? "r" : "-";
    group += (permissions & S_IWGRP) ? "w" : "-";
    group += (permissions & S_IXGRP) ? "x" : "-";

    std::string other;
    other += (permissions & S_IROTH) ? "r" : "-";
    other += (permissions & S_IWOTH) ? "w" : "-";
    other += (permissions & S_IXOTH) ? "x" : "-";

    return (is_dir ? "d" : "-") + user + group + other;
  }

  std::string get_file_type() const
  {
    switch (st_mode & S_IFMT)
    {
    case S_IFBLK:
      return "block device";
    case S_IFCHR:
      return "character device";
    case S_IFDIR:
      return "directory";
    case S_IFIFO:
      return "FIFO/pipe";
#ifndef _WIN32
    case S_IFLNK:
      return "symlink";
    case S_IFSOCK:
      return "socket";
#endif
    case S_IFREG:
      return "regular file";

    default:
      return "unknown";
    }
  }

  // Returns the creation time.
  // On Linux, if stx_btime is available use it; otherwise, fall back to stx_ctime.
  __time_t creation_time() const
  {
#if PLATFORM == 2 && !defined(NO_STATX)
    if (file_statx.stx_mask & STATX_BTIME)
      return file_statx.stx_btime.tv_sec;
    else
      return file_statx.stx_ctime.tv_sec;
#elif PLATFORM == 3
    return file_stat.st_birthtime;
#else
    return file_stat.st_ctime;
#endif
  }

  // Returns the modification time.
  const __time_t modification_time() const
  {
#if PLATFORM == 2 && !defined(NO_STATX)
    return file_statx.stx_mtime.tv_sec;
#else
    return file_stat.st_mtime;
#endif
  }

  // Returns the last access time.
  const __time_t access_time() const
  {
#if PLATFORM == 2 && !defined(NO_STATX)
    return file_statx.stx_atime.tv_sec;
#else
    return file_stat.st_atime;
#endif
  }
};
