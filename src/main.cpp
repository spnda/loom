#include <iostream>
#include <filesystem>

#include "shortcut.hpp"

namespace fs = std::filesystem;

/**
 * Adds a reference to %AppData%\\Loom to the user's PATH variable.
 * Requires administrator privilges.
 */
bool addToPath() {
    // They registry values.
    static const char registry_key[] = "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment";
    static const char registry_value[] = "Path";

    // Get the base environment variable.
    HKEY key;
    HRESULT openResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, registry_key, 0, KEY_ALL_ACCESS, std::addressof(key));
    if (openResult == 5) {
        std::cout << "Failed to open environment variables. Missing permissions, rerun this program with admin rights." << std::endl;
        return false;
    } else if (openResult == 0) {
        // There's a maximum of 32767 characters in environment variables.
        BYTE buffer[32767] = {};
        DWORD buffer_size = sizeof(buffer);

        HRESULT queryResult = RegQueryValueEx(key, registry_value, NULL, nullptr, buffer, std::addressof(buffer_size));
        if (queryResult != 0) return false;

        // The "Path" env variable is now in "buffer".
        std::string str(reinterpret_cast<char*>(buffer), buffer_size - 1);

        // The new path to add to the environment variable.
        fs::path appData = Shortcut::GetAppDataPath();
        appData /= "Loom";

        // Check if it's already added.
        if (str.find(appData.string()) == std::string::npos) {
            auto last_char = str.back();
            if (strcmp(&last_char, ";") != 0) {
                // Add a new ; to delimit the variables.
                str += ";";
            } else if (strcmp(&last_char, "?") != 0) {
                // In some cases, the environment string ends with a question-mark.
                str = str.substr(0, str.size() - 1);
            }

            // We now write our path to the end of the 
            str += appData.string();
            str += ";";

            HRESULT setResult = RegSetValueEx(key, registry_value, (DWORD)0, REG_EXPAND_SZ, (BYTE*)str.c_str(), strlen(str.c_str()));
        } else {
            std::cout << "Already initialized!" << std::endl;
            return false;
        }
    } else {
        std::cout << openResult << std::endl;
    }

    RegFlushKey(key);
    RegCloseKey(key);

    // https://docs.microsoft.com/en-us/windows/win32/procthread/environment-variables
    // That microsoft doc says we should broadcast this message.
    DWORD_PTR res;
    SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)TEXT("AddLoomToPath"), 0, 100, &res);

    return true;
}

int main(int argc, char* argv[]) {
    if (argc == 1) return -1;

    if (std::strcmp(argv[1], "add") == 0) {
        std::string path(argv[2]);
        Shortcut* sc = new Shortcut(fs::path(path));

        CreateLinkResult result = sc->CreateLink();
        if (result == CLR_SUCCESS) {
            std::cout << "Successfully created link." << std::endl;
        } else {
            std::cout << "Failed to create link. Not valid path or it might already exist?" << std::endl;
        }
    } else if (std::strcmp(argv[1], "list") == 0) {
        std::vector<Shortcut*> links = Shortcut::GetAllLinks();
        if (links.size() == 0) {
            std::cout << "No links have been created yet. Use <loom add> to add more." << std::endl;
        } else {
            for (auto& link : links) {
                std::cout << link->toString() << std::endl;
            }
        }
    } else if (std::strcmp(argv[1], "init") == 0) {
        if (addToPath()) {
            std::cout << "Loom has been initialized!" << std::endl;
        }
    } else {
        std::cout << "Unknown command. Use <loom add|list|init> instead.";
    }

    return 0;
}
