#include "project.hpp"

#include <cctype>
#include <filesystem>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

namespace novi {
namespace {

bool valid_project_name(const std::string& name) {
    if (name.empty() || name == "." || name == "..") return false;
    for (const unsigned char character : name) {
        if (!std::isalnum(character) && character != '-' && character != '_') return false;
    }
    return true;
}

bool write_file(const std::filesystem::path& path, const std::string& contents, std::string& error) {
    std::ofstream output(path);
    if (!output) {
        error = "Could not create " + path.string();
        return false;
    }
    output << contents;
    if (!output) {
        error = "Could not finish writing " + path.string();
        return false;
    }
    return true;
}

}  // namespace

bool create_project(const std::string& name, std::string& error) {
    namespace fs = std::filesystem;
    if (!valid_project_name(name)) {
        error = "Project name must contain only letters, digits, '-' or '_'.";
        return false;
    }

    const fs::path root(name);
    std::error_code filesystem_error;
    if (fs::exists(root, filesystem_error)) {
        error = "Path already exists: " + root.string();
        return false;
    }
    if (!fs::create_directories(root / "src", filesystem_error) || filesystem_error) {
        error = "Could not create project directory: " + root.string();
        return false;
    }

    const std::vector<std::pair<fs::path, std::string>> files = {
        {root / "src" / "main.nvi",
            "import {novi} from {novi};\n\nconsole.print('Hello from Novi!');\n"},
        {root / "novi.toml",
            "name = \"" + name + "\"\nversion = \"0.1.0\"\nentry = \"src/main.nvi\"\n"},
        {root / "README.md",
            "# " + name + "\n\nA Novi project.\n\n## Run\n\n```sh\nnovi src/main.nvi\n```\n"},
        {root / ".gitignore", "build/\n.novi-cache/\n"}
    };

    for (const auto& file : files) {
        if (!write_file(file.first, file.second, error)) return false;
    }
    return true;
}

}  // namespace novi
