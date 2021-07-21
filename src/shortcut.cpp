#include <iostream>
#include <system_error>

#include "shortcut.hpp"

using namespace std::literals;

// namespace fs = std::filesystem;

/**
 * Replaces a string inside another string with a string.
 * Thanks to https://stackoverflow.com/a/3418285/9156308.
 */
static inline bool replace(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if (start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

/**
 * Replaces all occurences of "from" in "str" to "to".
 * Thanks to https://stackoverflow.com/a/29752943/9156308.
 */
static inline void replaceAll(std::string& source, const std::string& from, const std::string& to) {
    std::string newString;
    newString.reserve(source.length());

    size_t lastPos = 0;
    size_t findPos;

    while((findPos = source.find(from, lastPos)) != std::string::npos) {
        newString.append(source, lastPos, findPos - lastPos);
        newString += to;
        lastPos = findPos + from.length();
    }

    // Care for the rest after last occurrence
    newString += source.substr(lastPos);

    source.swap(newString);
}

Shortcut* Shortcut::FromFile(std::string link_name) {
    fs::path path = GetAppDataPath();
    path /= "Loom";
    path /= link_name;

    // Read the first line of the file.
    std::ifstream link(path);
    std::string dummy_line;
    getline(link, dummy_line);
    std::string link_destination;
    getline(link, link_destination);

    // We store the destination in the first line
    // of the file as a batch comment.
    replace(link_destination, "rem ", "");
    Shortcut* sh = new Shortcut(link_destination);
    sh->link_location = path;
    return sh;
}

EditLinkResult Shortcut::CreateLink() const {
    // Check if the link already exists.
    // Possibly a naming collision.
    if (fs::exists(this->link_location)) return ELR_ALREADY_EXISTS;

    // Create the directory, if not already.
    fs::create_directories(this->link_location.parent_path());

    // Check if the source actually exists.
    if (!fs::exists(this->link_destination)) return ELR_INVALID_SOURCE;

    // Check if the link destination is not a directory.
    if (fs::is_directory(this->link_destination)) return ELR_INVALID_SOURCE;

    // Don't allow other programs called "loom".
    if (strcmp(this->link_location.filename().string().c_str(), "loom.cmd") == 0)
        return ELR_INVALID_SOURCE;

    // Create the file and add its contents.
    std::ofstream file(this->link_location);
    file << "@echo off" << std::endl;
    file << "rem \"" << this->link_destination.string() << "\"" << std::endl;
    file << "call \"" << this->link_destination.string() << "\" %*" << std::endl;
    file.close();

    return ELR_SUCCESS;
}

EditLinkResult Shortcut::RenameLink(std::string new_name) {
    // We don't want to allow links to be renamed to loom.
    if (new_name == "loom"sv) return ELR_INVALID_SOURCE;

    fs::path new_path = this->link_location;
    new_path.replace_filename(new_name + ".cmd");

    // We don't want to rename this to something that already exists.
    if (fs::exists(new_path)) return ELR_ALREADY_EXISTS;

    // Rename the file.
    fs::rename(this->link_location, new_path);
    this->link_location = new_path;
    
    return ELR_SUCCESS;
}

EditLinkResult Shortcut::ChangeDestination(fs::path destination) {
    // Check if the source actually exists.
    if (!fs::exists(destination)) return ELR_INVALID_SOURCE;

    // Check if the link destination is not a directory.
    if (fs::is_directory(destination)) return ELR_INVALID_SOURCE;

    // Read the entire file as a std::string.
    std::ifstream file(this->link_location);
    std::stringstream ss;
    ss << file.rdbuf();
    file.close();

    std::string content = ss.str();
    // We can still use this->link_destination, as it has not been updated yet.
    replaceAll(content, this->link_destination.string(), "\"" + destination.string() + "\"");
    this->link_destination = destination;

    std::ofstream outFile(this->link_location);
    outFile << content;
    outFile.close();

    return ELR_SUCCESS;
}

std::string Shortcut::ToString() {
    return this->GetFileName() + " -> " + this->link_destination.string();
}

std::vector<Shortcut*> Shortcut::GetAllLinks() {
    std::vector<Shortcut*> ret = {};
        
    fs::path base_folder = GetAppDataPath();
    base_folder /= "Loom"; // Our main folder.

    // Read all files in our folder.
    for (const auto& entry : fs::directory_iterator(base_folder)) {
        fs::path path = entry.path();
        
        // Ignore files that don't have an extension or don't end with ".cmd".
        if (!path.has_extension() || std::strcmp(path.extension().string().c_str(), ".cmd") != 0)
            continue;

        ret.push_back(Shortcut::FromFile(path.filename().string()));
    }

    return ret;
}

fs::path Shortcut::ConvertFileName(fs::path filename) {
    return filename.replace_extension("cmd");
}
