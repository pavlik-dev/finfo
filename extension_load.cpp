// extension_load.cpp
#include "Extension.cpp"
typedef Extension* (*create_extension_t)();
typedef void (*destroy_extension_t)(Extension*);

// Helper function to load an extension
shared_ptr<Extension> load_extension(const string& libpath) {
    cout << "Loading " << libpath << endl;
    // Load the shared library
    void* handle = dlopen(libpath.c_str(), RTLD_LAZY);
    if (!handle) {
        cerr << "Error loading library: " << dlerror() << endl;
        return nullptr;
    }

    // Load the 'create_extension' symbol
    create_extension_t create_extension = (create_extension_t) dlsym(handle, "create_extension");
    if (!create_extension) {
        cerr << "Error loading symbol: " << dlerror() << endl;
        dlclose(handle);
        return nullptr;
    }

    // Create the extension instance
    Extension* ext = create_extension();
    unique_ptr<Extension> uext = unique_ptr<Extension>(ext);
    shared_ptr<Extension> sext = std::move(uext);
    return sext; // Manage the pointer with a unique_ptr
}