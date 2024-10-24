#ifndef EXC_CPP
#define EXC_CPP
class Exception {
private:
	std::string _what;
public:
	Exception(std::string what) : _what(what) {}
	std::string what() {
		return this->_what;
	}
};
#endif