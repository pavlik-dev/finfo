// DON'T INCLUDE THIS!
// YOU NEED TO COMPILE THIS SEPARATELY, AS A .so FILE!!!
// g++ -o exts/name.ext -fPIC -shared name.cpp

// #include <opencv2/core.hpp>
// #include <opencv2/imgcodecs.hpp>
#include "Extension.cpp"
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include "mime.cpp"

struct ImgDim
{
    unsigned int x;
    unsigned int y;
};

bool is_png(const std::string& filepath) {
    ifstream input (filepath, ios::binary);
    if (input.is_open()) {
        input.seekg(0, ios::beg);
        unsigned char magic[8] = {0};
        input.read((char*)magic, sizeof(magic));

        const unsigned char png_magic[8] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
        return memcmp(magic, png_magic, sizeof(magic)) == 0;
    }
    return false;
}

struct ImgDim get_png_image_dimensions(const std::string& file_path)
{
    struct ImgDim result;
    unsigned char buf[8];
    
    std::ifstream in(file_path);
    in.seekg(16);
    in.read(reinterpret_cast<char*>(&buf), 8);

    result.x = (buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + (buf[3] << 0);
    result.y = (buf[4] << 24) + (buf[5] << 16) + (buf[6] << 8) + (buf[7] << 0);

    return result;
}

// Example Image Extension
class MimeExtension : public Extension {
public:
    MimeExtension() : Extension("int.mime_ext") {}

    bool is_compatible(const std::string& filepath) override {
        // Check if the file is an image (for simplicity, we'll just check the extension)
        return is_png(filepath);
    }

    Field get_info(const std::string& filepath) override {
        // Stub: In a real scenario, you would get image dimensions, etc.
        // std::string image_path = filepath;
        // cv::Mat img = cv::imread(image_path, cv::IMREAD_COLOR);
     
        // if(img.empty())
        // {
        //     throw Exception("Cannot open this image :(");
        // }
        // return Field("res", "Resolution", to_string(img.cols)+"x"+to_string(img.rows));
        auto res = get_png_image_dimensions(filepath);
        return Field("res", "Resolution", to_string(res.x)+"x"+to_string(res.y));
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
