#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <vector>

#ifdef _WIN32
#define STAT _stat
#define DELIM "\\"
#else
#define STAT stat
#define DELIM "/"
#endif

#ifdef WIN32
#define realpath(N, R) _fullpath((R), (N), _MAX_PATH)
#endif

#ifndef _MAX_PATH
#define _MAX_PATH 255
#endif

using namespace std;

class NotFoundException
{
public:
    NotFoundException(const string &_what) : _what(_what) {}
    string what() const
    {
        return _what + " doesn't exist.";
    }

private:
    string _what;
};

class StatFailedException
{
public:
    StatFailedException(const string &_what) : _what(_what) {}
    string what() const
    {
        return "stat/lstat failed: " + _what;
    }

private:
    string _what;
};

class WinCallFailedException
{
public:
    WinCallFailedException(const string &_what) : _what(_what) {}
    string what() const
    {
        return "stat/lstat failed: " + _what;
    }

private:
    string _what;
};

bool __file_exists(const string &_name)
{
    struct STAT buffer;
    return STAT(_name.c_str(), &buffer) == 0;
}

class File {
public:
    string file_name;
    string abs_path;
    struct STAT file_stat;

    // Constructor
    File(const string& file) {
        this->file_name = file;

        if (!__file_exists(file)) {
            throw NotFoundException(file);
        }

        char* temp = (char*)malloc(_MAX_PATH);
        if (realpath(file.c_str(), temp) == NULL) {
            perror("realpath");
            throw StatFailedException("Failed to resolve absolute path.");
        }
        this->abs_path = string(temp);
        free(temp);

        if (STAT(file.c_str(), &file_stat) != 0) {
            perror(("stat error at file " + file).c_str());
            throw StatFailedException(file);
        }
    }

    File() {}
};
