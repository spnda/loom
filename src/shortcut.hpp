#pragma once

#include <sstream>
#include <fstream>
#include <filesystem>

#include "registry.hpp"

namespace fs = std::filesystem;

enum EditLinkResult {
    ELR_SUCCESS = 0,
    ELR_ALREADY_EXISTS = 1,
    ELR_INVALID_SOURCE = 2,
};

class Shortcut {
public:
    /**
     * @brief The location of where the program/executable
     *        to link is.
     */
    fs::path link_destination;

    /**
     * @brief A new shortcut that links to "path".
     */
    Shortcut(fs::path path) : link_destination(path) {
        // Create the path to the shortcut.
        // We use "AppData/Roaming/Loom/filename.cmd"
        link_location = GetAppDataPath();
        link_location /= "Loom"; // Our main folder.
        link_location /= this->ConvertFileName(path.filename());

        // Make sure our folder exists
        fs::create_directories(link_location.parent_path());
    };

    /**
     * @brief Reads a shortcut file and creates a instance based on
     *        its contents.
     * 
     * @param link_name This should meerely be the name of the link file,
     *                  and not its path.
     */
    static Shortcut* FromFile(std::string link_name);

    /**
     * @brief Creates a new shortcut file.
     * 
     * @return True if the file was created successfully.
     *         False if the file exists already or there was
     *         some error during writing.
     */
    EditLinkResult CreateLink() const;

    /**
     * @brief Renames this link, keeping the destination the same.
     * 
     * @param new_name Should be the new file name, not the actual
     *                 path to the new link.
     */
    EditLinkResult RenameLink(std::string new_name);

    /**
     * @brief Returns a string representation of this link file.
     */
    std::string ToString();

    /**
     * @brief Get the name of the link file (without the extension).
     */
    std::string GetFileName() {
        return this->link_location.filename().replace_extension("").string();
    }

    /**
     * @brief Gets a list of all links that have been created.
     * 
     * @return A vector with the full path to each link.
     */
    static std::vector<Shortcut*> GetAllLinks();

private:
    /**
     * @brief The location of the shortcut linking to the
     *        program.
     */
    fs::path link_location;

    /**
     * @brief Converts the filename to a shortcut filename.
     */
    static fs::path ConvertFileName(fs::path filename);
};
