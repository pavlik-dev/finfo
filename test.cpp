#include <iostream>
#include <string>
using namespace std;
string dirnameOf(const string &fname) {
  size_t pos = fname.find_last_of("\\/");
  return (string::npos == pos) ? "" : fname.substr(0, pos);
}
inline string get_extension(const string &filename) {
  return filename.substr(filename.find_last_of(".") + 1);
}
int main(){
  cout << dirnameOf("test/file.txt") << endl;
  cout << get_extension("test/file") << endl;
}
