#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <vector>
#ifndef _WIN32
#define STAT stat
#define DELIM "/"
#else
#define STAT _stat
#define DELIM "\\"
#endif

#ifdef WIN32
#define realpath(N,R) _fullpath((R),(N),_MAX_PATH)
#endif

#ifndef _MAX_PATH
#define _MAX_PATH 255
#endif

using namespace std;

inline bool __file_exists(const string& name) {
    struct STAT buffer;
    return STAT(name.c_str(), &buffer) == 0;
}

enum Stat {
    _STAT,
    LSTAT
};

class NotFoundException {
public:
    NotFoundException(const string& _what) : _what(_what) {}
    string what() const {
        return _what + " doesn't exist.";
    }
private:
    string _what;
};

class StatFailedException {
public:
    StatFailedException(const string& _what) : _what(_what) {}
    string what() const {
        return "stat/lstat failed: " + _what;
    }
private:
    string _what;
};

inline string basename(string path, bool is_dir = false)
{
	return (path.substr(path.find_last_of(DELIM) + 1)) + ((is_dir) ? DELIM : "");
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

        vector<char> temp(_MAX_PATH);
        if (realpath(file.c_str(), temp.data()) == nullptr) {
            perror("realpath");
            throw StatFailedException("Failed to resolve absolute path.");
        }
        this->abs_path = string(temp.data());

        if (STAT(file.c_str(), &file_stat) != 0) {
            perror(("stat error at file " + file).c_str());
            throw StatFailedException(file);
        }
    }

    File() {}
};
