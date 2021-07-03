#include <iostream>
#include <system_error>

#include "shortcut.hpp"

// namespace fs = std::filesystem;

/**
 * Replaces a string inside another string with a string.
 * Thanks to https://stackoverflow.com/a/3418285/9156308.
 */
bool replace(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if (start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

Shortcut* Shortcut::FromFile(fs::path link_file) {
    // Read the first line of the file.
    std::ifstream link(link_file);
    std::string dummy_line;
    getline(link, dummy_line);
    std::string link_destination;
    getline(link, link_destination);

    // We store the destination in the first line
    // of the file as a batch comment.
    replace(link_destination, "rem ", "");
    Shortcut* sh = new Shortcut(link_file);
    sh->link_destination = link_destination;
    return sh;
}

CreateLinkResult Shortcut::CreateLink() {
    // Check if the link already exists.
    // Possibly a naming collision.
    if (fs::exists(this->link_location)) return CLR_ALREADY_EXISTS;

    // Create the directory, if not already.
    fs::create_directories(this->link_location.parent_path());

    // Check if the source actually exists.
    if (!fs::exists(this->link_destination)) return CLR_INVALID_SOURCE;

    // Check if the link destination is not a directory.
    if (fs::is_directory(this->link_destination)) return CLR_INVALID_SOURCE;

    // Don't allow other programs called "loom".
    if (strcmp(this->link_location.filename().string().c_str(), "loom.cmd") == 0)
        return CLR_INVALID_SOURCE;

    // Create the file and add its contents.
    std::ofstream file(this->link_location);
    file << "@echo off" << std::endl;
    file << "rem \"" << this->link_destination.string() << "\"" << std::endl;
    file << "call \"" << this->link_destination.string() << "\" %*" << std::endl;
    file.close();

    return CLR_SUCCESS;
}

std::string Shortcut::toString() {
    return this->link_location.filename().replace_extension("").string() + " -> " + this->link_destination.string();
}

const std::vector<Shortcut*> Shortcut::GetAllLinks() {
    std::vector<Shortcut*> ret = {};
        
    fs::path base_folder = Shortcut::GetAppDataPath();
    base_folder /= "Loom"; // Our main folder.

    // Read all files in our folder.
    for (const auto& entry : fs::directory_iterator(base_folder)) {
        fs::path path = entry.path();
        
        // Ignore files that don't have an extension or don't end with ".cmd".
        if (!path.has_extension() || std::strcmp(path.extension().string().c_str(), ".cmd") != 0)
            continue;

        ret.push_back(Shortcut::FromFile(path));
    }

    return ret;
}

const fs::path Shortcut::ConvertFileName(fs::path filename) {
    return filename.replace_extension("cmd");
}

const fs::path Shortcut::GetAppDataPath() {
    // Get the AppData folder.
    PWSTR path = 0; // <-- This is a wchar_t* holding the path in unicode.
    auto result = SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_CREATE | KF_FLAG_DONT_UNEXPAND | KF_FLAG_NO_ALIAS, NULL, &path);

    if (!SUCCEEDED(result)) {
        std::cout << std::system_category().message(result) << std::endl;
    }

    // Convert our PWSTR into a stringstream.
    std::wstringstream ss;
    ss << path;

    // We have to free the memory of the path ourselfs.
    CoTaskMemFree(static_cast<void*>(path));

    return fs::path(ss.str());
}
