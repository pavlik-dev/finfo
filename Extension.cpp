#include <string>
#include "Field.cpp"

class Extension {
public:
    std::string ext_id;

    Extension(std::string ext_id) : ext_id(ext_id) {}

    virtual bool is_compatible(const std::string& filepath) = 0;
    virtual Field get_info(const std::string& filepath) = 0;

    virtual ~Extension() = default; // Ensure proper cleanup
};

class Exception {
private:
	std::string _what;
public:
	Exception(std::string what) : _what(what) {}
	std::string what() {
		return this->_what;
	}
};