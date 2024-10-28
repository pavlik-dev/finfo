// DON'T INCLUDE THIS!
// YOU NEED TO COMPILE THIS SEPARATELY, AS A .so FILE!!!
// g++ -o exts/name.ext -fPIC -shared name.cpp

#define EXT_ID "pyt.mime_ext"

#include "Extension.cpp"
#include <iostream>
#include <string>
#include "mime.cpp"

// Example Image Extension
class MimeExtension : public Extension {
public:
    MimeExtension() : Extension(EXT_ID) {}

    bool is_compatible(const std::string& filepath) override {
        return true;
    }

    Field get_info(const std::string& filepath) override {
        return Field("mime", "MIME", get_mime(filepath));
    }
};

// Factory function for creating the extension
extern "C" Extension* create_extension() {
    return new MimeExtension();
}

// Factory function for destroying the extension
extern "C" void destroy_extension(Extension* ext) {
    delete ext;
}

int main() {
    cout << "This is a finfo extension." << endl;
    cout << "https://gitea.com/pavliktt/finfo" << endl;
}
