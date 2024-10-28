// file.cpp

inline bool __file_exists(const string& name) {
  struct stat buffer;
  return lstat(name.c_str(), &buffer) == 0;
}

enum Stat {
  STAT,
  LSTAT
};

class NotFoundException {
public:
  NotFoundException(string _what) : _what(_what) {}
  string what() {
    return this->_what+" doesn't exists.";
  }
private:
  string _what = "This file";
};

class StatFailedException {
public:
  StatFailedException(string _what) : _what(_what) {}
  string what() {
    return "stat/lstat failed: "+this->_what;
  }
private:
  string _what = "This file";
};

class File {
public:
  string file_name;
  string abs_path;
  struct stat file_stat;
  // Constructor
  File(string file, Stat _stat = Stat::LSTAT) {
    this->file_name = file;
    if (!__file_exists(file)) {
      throw new NotFoundException(file);
    }
    char* temp = (char*)malloc(4097);
    temp[4096] = '\0';
    if (realpath(file.c_str(), temp) == nullptr) {
      perror("realpath");
    }
    string abspath(temp);
    free(temp);
    this->abs_path = abspath;
    struct stat filestat;
    if (((_stat == Stat::LSTAT) ? lstat : stat)(file.c_str(), &file_stat) != 0) {
      string errdesc = "stat error at file "+file;
      perror(errdesc.c_str());
      return;
    }
    this->file_stat = filestat;
    return; 
  }
};