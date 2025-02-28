#ifdef PLATFORM
#elif defined(__CYGWIN__) || defined(_WIN32) || defined(_WIN64)
#define PLATFORM 1 // 1 for WIN32
#elif defined(__linux__) || defined(__linux)
#define PLATFORM 2 // 2 for LINUX
#elif defined(__MACH__)
#define PLATFORM 3 // 3 for MACOSX
#elif defined(__APPLE__)
#define PLATFORM 4 // 4 for MACOSCLASSIC
#elif defined(__FreeBSD__)
#define PLATFORM 5 // 5 for FREEBSD
#elif defined(unix) || defined(__unix) || defined(__unix__)
#define PLATFORM 6 // 6 for UNIX
#elif defined(__ANDROID__)
#define PLATFORM 7 // 7 for ANDROID
#else
#error Unsupported platform!
#endif
