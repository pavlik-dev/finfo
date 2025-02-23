#ifndef MIME_CPP
#define MIME_CPP

#if !defined(NO_MIME) && !defined(_WIN32)
#include <magic.h>

std::string get_mime(const std::string& filename)
{
  const char* file_path = filename.c_str();
  const auto magic_cookie = static_cast<const magic_t>(magic_open(MAGIC_MIME_TYPE));
  if (magic_cookie == nullptr) {
    std::cerr << "Unable to initialize libmagic\n";
    return "";
  }

  // Load definitions from the mime types database
  if (magic_load(magic_cookie, nullptr) != 0) {
    std::cerr << "Unable to load database definitions\n";
    magic_close(magic_cookie);
    return "";
  }

  // Determines the MIME type of the file
  const char *mime_type = magic_file(magic_cookie, file_path);
  if (mime_type == nullptr) {
    std::cerr << "Unable to determine the MIME type of the file\n";
    magic_close(magic_cookie);
    return "";
  }

  std::string result(mime_type);

  // Close libmagic
  magic_close(magic_cookie);
  return result;
}
#else
#include "mime_by_extension.cpp"
#endif

#endif
