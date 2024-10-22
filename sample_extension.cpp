// DON'T INCLUDE THIS!
// YOU NEED TO COMPILE THIS SEPARATELY, AS A .so FILE!!!

#include "Extension.cpp"
#include <iostream>
#include <string>

// Example Image Extension
class ImageExtension : public Extension {
public:
    ImageExtension() : Extension("img_ext") {}

    bool is_compatible(const std::string& filepath) override {
        // Check if the file is an image (for simplicity, we'll just check the extension)
        return filepath.find(".png") != std::string::npos || filepath.find(".jpg") != std::string::npos;
    }

    Field get_info(const std::string& filepath) override {
        // Stub: In a real scenario, you would get image dimensions, etc.
        Field info("Image Information", "");
        info.add_field(Field("Resolution", "1920x1080"));
        info.add_field(Field("Color Depth", "24-bit"));

        return info;
    }
};

// Factory function for creating the extension
extern "C" Extension* create_extension() {
    return new ImageExtension();
}

// Factory function for destroying the extension
extern "C" void destroy_extension(Extension* ext) {
    delete ext;
}
