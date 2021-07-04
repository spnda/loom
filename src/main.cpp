#include <iostream>
#include <filesystem>

#include "shortcut.hpp"

using namespace std::literals;

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    if (argv[1] == "add"sv) {
        std::string path(argv[2]);
        Shortcut* sc = new Shortcut(fs::path(path));

        CreateLinkResult result = sc->CreateLink();
        if (result == CLR_SUCCESS) {
            std::cout << "Successfully created link." << std::endl;
        } else {
            std::cout << "Failed to create link. Not valid path or it might already exist?" << std::endl;
        }
    } else if (argv[1] == "list"sv) {
        std::vector<Shortcut*> links = Shortcut::GetAllLinks();
        if (links.size() == 0) {
            std::cout << "No links have been created yet. Use <loom add> to add more." << std::endl;
        } else {
            for (auto& link : links) {
                std::cout << link->toString() << std::endl;
            }
        }
    } else if (argv[1] == "init"sv) {
        if (AddLoomToPath()) {
            std::cout << "Loom has been initialized!" << std::endl;
        }
    } else {
        std::cout << "Unknown command. Use <loom add|list|init> instead.";
    }

    return 0;
}
