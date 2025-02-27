#define TAB "  "
#define COLOR 0

#include <iostream>
#include <memory>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <sstream>
#include <getopt.h>
#include <ctime>
#include <iomanip>

/* Optional headers */
#ifndef NO_MNTENT
#include <mntent.h>
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

int print_usage(string program_name)
{
  cerr << "Usage:" << endl;
  cerr << TAB << program_name << " [-d] <files>\n"
       << endl;
  cerr << TAB << "-d  --  Detailed view, fields f" << "rom `struct stat` (see 'man stat.3')" << endl;
  return 1;
}

int main(int argc, char *argv[])
{
  if (argc < 2)
    return print_usage(argv[0]);

  bool detailed = false;

  vector<string> files;
  bool _files = false;
  for (int i = 1; i < argc; ++i)
  {
    if (argv[i] == string("-d") && !_files)
    {
      detailed = true;
      _files = true;
      continue;
    }
    files.push_back(string(argv[i]));
  }

  if (files.size() < 1)
    return print_usage(argv[0]);

  int counter = 0;
  for (const string &file : files)
  {
    counter++;
    File file_obj;
    string abspath = file;
    bool is_dir = false;
    Field start(File::basename(abspath), "");
    try
    {
      file_obj = File(file);
    }
    catch (const FileException &e)
    {
      start.addField(Field("Error", string(e.what())));
      cout << start.print(TAB) << flush;
      continue;
    }

    abspath = file_obj.abs_path;

    string file_type = file_obj.get_file_type();

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
      DirContents insides = file_obj.get_files_in_directory();

      ostringstream files_str, dirs_str;

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
        string mount_points = "";
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
          mount_points += ((mount_points != "") ? ", " : "") + string(ent->mnt_dir);
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
      start.emplaceField("Parent device ID", to_string(file_obj.st_dev));
      start.emplaceField("Inode", to_string(file_obj.st_ino));
      start.emplaceField("Hard links", to_string(file_obj.st_nlink));
      start.emplaceField("Owner", to_string(file_obj.st_uid) + ", group " +
                                      to_string(file_obj.st_gid));
      start.emplaceField("Special device ID", to_string(file_obj.st_rdev));
      start.emplaceField("Size in bytes", to_string(file_obj.st_size));
      start.emplaceField("FS block size", to_string(file_obj.st_blksize));
      start.emplaceField("Total blocks", to_string(file_obj.st_blocks));

      start.addDelimiter();
    }

    char buffer[8];
    snprintf(buffer, sizeof(buffer), " (%3o)", file_obj.permissions);
    string _perms(buffer);
    start.emplaceField(
        "Permissions", file_obj.get_permissions() + _perms);

    start.addDelimiter();

    /* Date fields */
    struct tm *timeinfo;
    std::ostringstream oss;

    time_t created = file_obj.creation_time();
    timeinfo = localtime(&created);
    oss << std::put_time(timeinfo, " %Y-%m-%d %H:%M:%S")
    #if PLATFORM == 2
        << (detailed ? "." + to_string(file_obj.file_statx.stx_btime.tv_nsec) : "");
    #else
        ;
    #endif
    start.emplaceField("Created", oss.str());

    oss.str("");
    oss.clear();

    time_t last_opened = file_obj.access_time();
    timeinfo = localtime(&last_opened);

    oss << std::put_time(timeinfo, "  %Y-%m-%d %H:%M:%S")
    #if PLATFORM == 2
        << (detailed ? "." + to_string(file_obj.file_statx.stx_atime.tv_nsec) : "");
    #else
        ;
    #endif
    start.emplaceField("Opened", oss.str());

    oss.str("");
    oss.clear();
    time_t modified = file_obj.modification_time();
    timeinfo = localtime(&modified);
    oss << std::put_time(timeinfo, "%Y-%m-%d %H:%M:%S")
    #if PLATFORM == 2
        << (detailed ? "." + to_string(file_obj.file_statx.stx_mtime.tv_nsec) : "");
    #else
        ;
    #endif
    start.emplaceField("Modified", oss.str());

    start.name = File::basename(file_obj.file_name) + (is_dir ? "/" : "");
    cout << start.print(TAB) << flush;
  }
  return 0;
}
