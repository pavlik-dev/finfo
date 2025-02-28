#ifndef TOSTRING
#define TOSTRING
#include <string>
#include <sstream>
namespace std
{
  template <typename T>
  std::string to_string(const T &value)
  {
    std::ostringstream oss;
    oss << value;
    return oss.str();
  }
}
#endif