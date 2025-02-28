#define TAB "  "
#define COLOR 0
#include "platform.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

/* Optional headers */
#if !defined(NO_MNTENT) && PLATFORM != 1
#include <mntent.h>
#endif

#include "Field.cpp"
#include "File.cpp"
#include "mime.cpp"
#include "to_string.cpp"
#include "put_date.cpp"

// Maybe move this to string_ops.cpp?
std::string trim(const std::string &str, const std::string &whitespace = " \t\n")
{
  const std::size_t strBegin = str.find_first_not_of(whitespace);
  if (strBegin == std::string::npos)
    return ""; // no content

  const std::size_t strEnd = str.find_last_not_of(whitespace);
  const std::size_t strRange = strEnd - strBegin + 1;

  return str.substr(strBegin, strRange);
}

std::string exec(std::string cmd)
{
  FILE *pipe = popen(cmd.c_str(), "r");
  if (!pipe)
    return "ERROR";
  char buffer[128];
  std::string result = "";
  while (!feof(pipe))
  {
    if (fgets(buffer, 128, pipe) != NULL)
      result += buffer;
  }
  pclose(pipe);
  return result;
}

// https://stackoverflow.com/a/4643526
std::string escape_quotes(std::string str)
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

int print_usage(std::string program_name)
{
  std::cerr << "Usage:" << std::endl;
  std::cerr << TAB << program_name << " [-d] <files>\n"
            << std::endl;
  std::cerr << TAB << "-d  --  Detailed view, fields f" << "rom `struct stat` (see 'man stat.3')" << std::endl;
  return 1;
}

int main(int argc, char *argv[])
{
  if (argc < 2)
    return print_usage(argv[0]);

  bool detailed = false;

  std::vector<std::string> files;
  bool _files = false;
  for (int i = 1; i < argc; ++i)
  {
    if (argv[i] == std::string("-d") && !_files)
    {
      detailed = true;
      _files = true;
      continue;
    }
    files.push_back(std::string(argv[i]));
  }

  if (files.size() < 1)
    return print_usage(argv[0]);

  size_t counter = 0;
  for (counter = 0; counter < files.size(); ++counter)
  {
    std::string file = files[counter];
    File file_obj;
    std::string abspath = file;
    bool is_dir = false;
    Field start(File::basename(abspath), "");
    try
    {
      file_obj = File(file);
    }
    catch (const FileException &e)
    {
      start.addField(Field("Error", std::string(e.what())));
      std::cout << start.print(TAB) << std::flush;
      continue;
    }

    abspath = file_obj.abs_path;

    std::string file_type = file_obj.get_file_type();

    is_dir = file_obj.file_type == S_IFDIR;
    bool is_file = file_obj.file_type == S_IFREG;
    bool is_device = file_obj.file_type == S_IFBLK ||
                     file_obj.file_type == S_IFCHR;

    start.emplaceField("Type", file_type);
    if (is_file)
      start.emplaceField("MIME", get_mime(abspath));

    start.addDelimiter();

    if (is_dir)
    {
#ifndef NO_DIRENT
      DirContents insides = file_obj.get_files_in_directory();

      std::ostringstream files_str, dirs_str;

      if (insides.normal_files > 0)
        files_str << insides.normal_files;
      else
        files_str << "no";
      if (insides.hidden_files)
        files_str << " (+" << insides.hidden_files << " hidden)";

      if (insides.normal_dirs > 0)
        dirs_str << insides.normal_dirs;
      else
        dirs_str << "no";
      if (insides.hidden_dirs)
        dirs_str << " (+" << insides.hidden_dirs << " hidden)";

      start.emplaceField("Files", "      " + files_str.str());
      start.emplaceField("Directories", dirs_str.str());
#endif
    }
    else if (is_file)
    {
      start.emplaceField("Size", file_obj.readable_fs());
    }
    else if (is_device)
    {
#if PLATFORM != 1 && !defined(NO_MNTENT)
      // Mount point check
      struct mntent *ent;
      FILE *aFile;

      aFile = setmntent("/proc/mounts", "r");
      if (aFile == NULL)
      {
        perror("setmntent");
        start.emplaceField("Mount point: ", "N/A");
      }
      else
      {
        std::string mount_points = "";
        size_t total = 0;
        while (NULL != (ent = getmntent(aFile)))
        {
          if (ent->mnt_fsname == abspath)
            continue;
          if (total > 5)
          {
            mount_points += ", ...";
            break;
          } // Too much mount points!
          total += 1;
          mount_points += ((mount_points != "") ? ", " : "") + std::string(ent->mnt_dir);
        }

        if (mount_points != "")
          start.emplaceField("Mount point(s)", mount_points);
        endmntent(aFile);
      }
#endif
    }

    start.addDelimiter();

    if (detailed)
    {
      /* A bunch of stat values */
      start.emplaceField("Parent device ID", std::to_string(file_obj.st_dev));
      start.emplaceField("Inode", std::to_string(file_obj.st_ino));
#if PLATFORM != 1
      start.emplaceField("Hard links", std::to_string(file_obj.st_nlink));
      start.emplaceField("Owner", std::to_string(file_obj.st_uid) + ", group " +
                                      std::to_string(file_obj.st_gid));
      start.emplaceField("Special device ID", std::to_string(file_obj.st_rdev));
      start.emplaceField("Size in bytes", std::to_string(file_obj.st_size));
      start.emplaceField("FS block size", std::to_string(file_obj.st_blksize));
      start.emplaceField("Total blocks", std::to_string(file_obj.st_blocks));
#endif

      start.addDelimiter();
    }

    std::ostringstream oss;
    oss << " (" << std::setw(3) << std::setfill(' ') << std::oct << file_obj.permissions << ")";
    std::string _perms = oss.str();
    start.emplaceField(
        "Permissions", file_obj.get_permissions() + _perms);

    start.addDelimiter();

    /* Date fields */
    struct tm *timeinfo;
    oss.str("");
    oss.clear();

    time_t created = file_obj.creation_time();
    timeinfo = localtime(&created);
    oss << std::put_time(*timeinfo, " %Y-%m-%d %H:%M:%S")
#if PLATFORM == 2 && !defined(NO_STATX)
        << (detailed ? "." + std::to_string(file_obj.btime.tv_nsec) : "");
#else
        ;
#endif
    start.emplaceField("Created", oss.str());

    oss.str("");
    oss.clear();

    time_t last_opened = file_obj.access_time();
    timeinfo = localtime(&last_opened);

    oss << std::put_time(*timeinfo, "  %Y-%m-%d %H:%M:%S")
#if PLATFORM == 2 && !defined(NO_STATX)
        << (detailed ? "." + std::to_string(file_obj.atime.tv_nsec) : "");
#else
        ;
#endif
    start.emplaceField("Opened", oss.str());

    oss.str("");
    oss.clear();
    time_t modified = file_obj.modification_time();
    timeinfo = localtime(&modified);
    oss << std::put_time(*timeinfo, "%Y-%m-%d %H:%M:%S")
#if PLATFORM == 2 && !defined(NO_STATX)
        << (detailed ? "." + std::to_string(file_obj.mtime.tv_nsec) : "");
#else
        ;
#endif
    start.emplaceField("Modified", oss.str());

    start.name = File::basename(file_obj.abs_path) + (is_dir ? DELIM : "");
    std::cout << start.print(TAB) << std::flush;
  }
  return 0;
}
