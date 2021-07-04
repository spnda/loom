#pragma once

#ifdef _WIN32 // Registry stuff is Windows specific.

#include <sstream>
#include <iostream>
#include <filesystem>

// Windows
#include <windows.h>
#include <shlobj_core.h>

namespace fs = std::filesystem;

/**
 * Gets the path to the AppData folder on windows.
 */
const inline fs::path GetAppDataPath() {
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

/**
 * Adds a reference to %AppData%\\Loom to the user's PATH variable.
 * Requires administrator privilges.
 */
const inline bool AddLoomToPath() {
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
        fs::path appData = GetAppDataPath();
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

            HRESULT setResult = RegSetValueEx(key, registry_value, (DWORD)0, REG_EXPAND_SZ, (BYTE*)str.c_str(), (DWORD)strlen(str.c_str()));
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

#else
#error This program is not designed to run on Linux/OSX.
#endif // #ifdef _WIN32
