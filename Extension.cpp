class Extension {
public:
    std::string name;

    Extension(std::string ext_name) : name(ext_name) {}

    virtual bool is_compatible(const std::string& filepath) = 0;
    virtual Field get_info(const std::string& filepath) = 0;

    virtual ~Extension() = default; // Ensure proper cleanup
};