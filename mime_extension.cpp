// DON'T INCLUDE THIS!
// YOU NEED TO COMPILE THIS SEPARATELY, AS A .so FILE!!!
// g++ -o exts/name.ext -fPIC -shared name.cpp

#include "Extension.cpp"
#include <iostream>
#include <string>
#include "mime.cpp"

// Example Image Extension
class MimeExtension : public Extension {
public:
    MimeExtension() : Extension("int.mime_ext") {}

    bool is_compatible(const std::string& filepath) override {
        // Check if the file is an image (for simplicity, we'll just check the extension)
        return true;
    }

    Field get_info(const std::string& filepath) override {
        // Stub: In a real scenario, you would get image dimensions, etc.
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
