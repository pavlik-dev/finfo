#ifndef EXT_CPP
#define EXT_CPP
#include <string>
#include "Field.cpp"

class Extension {
public:
    std::string ext_id;
    std::string file_name = "unset"; // Don't set this.

    Extension(std::string ext_id) : ext_id(ext_id) {}

    virtual bool is_compatible(const std::string& filepath) = 0;
    virtual Field get_info(const std::string& filepath) = 0;

    virtual ~Extension() = default; // Ensure proper cleanup
};

#include "exception.cpp"
#endif