#pragma once

#include <sstream>
#include <fstream>
#include <filesystem>

// Windows
#include "winerror.h"
#include <windows.h>
#include <shlobj_core.h>

namespace fs = std::filesystem;

enum CreateLinkResult {
    CLR_SUCCESS = 0,
    CLR_ALREADY_EXISTS = 1,
    CLR_INVALID_SOURCE = 2,
};

class Shortcut {
public:
    /**
     * @brief A new shortcut that links to "path".
     */
    Shortcut(fs::path path) : link_destination(path) {
        // Create the path to the shortcut.
        // We use "AppData/Roaming/Loom/filename.cmd"
        link_location = this->GetAppDataPath();
        link_location /= "Loom"; // Our main folder.
        link_location /= this->ConvertFileName(path.filename());

        // Make sure our folder exists
        fs::create_directories(link_location.parent_path());
    };

    /**
     * @brief Reads a shortcut file and creates a instance based on
     *        its contents.
     */
    static Shortcut* FromFile(fs::path link_file);

    /**
     * @brief Creates a new shortcut file.
     * 
     * @return True if the file was created successfully.
     *         False if the file exists already or there was
     *         some error during writing.
     */
    CreateLinkResult CreateLink();

    /**
     * @brief Returns a string representation of this link file.
     */
    std::string toString();

    /**
     * @brief Gets a list of all links that have been created.
     * 
     * @return A vector with the full path to each link.
     */
    static const std::vector<Shortcut*> GetAllLinks();

private:
    /**
     * @brief The location of the shortcut linking to the
     *        program.
     */
    fs::path link_location;

    /**
     * @brief The location of where the program/executable
     *        to link is.
     */
    fs::path link_destination;

    /**
     * @brief Converts the filename to a shortcut filename.
     */
    static const fs::path ConvertFileName(fs::path filename);

    /**
     * @brief Gets the path to the AppData folder on windows.
     */
    static const fs::path GetAppDataPath();
};
