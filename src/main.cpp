#include <iostream>
#include <filesystem>

#include "shortcut.hpp"

using namespace std::literals;

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    if (argv[1] == "add"sv) {
        std::string path(argv[2]);
        Shortcut* sc = new Shortcut(fs::path(path));

        EditLinkResult result = sc->CreateLink();
        if (result == ELR_SUCCESS) {
            std::cout << "Successfully created link." << std::endl;
        } else {
            std::cout << "Failed to create link. Not valid path or it might already exist?" << std::endl;
        }
    } else if (argv[1] == "list"sv) {
        std::vector<Shortcut*> links = Shortcut::GetAllLinks();
        if (links.size() == 0) {
            std::cout << "No links have been created yet. Use <loom add> to add more." << std::endl;
        } else {
            if (links.size() == 1) {
                std::cout << links.front()->ToString() << std::endl;
            } else {
                // For nice formatting, we'll fix the arrows to be the same
                // offset on each line.
                size_t max_length = 5;

                // Calculate max link name length.
                for (auto& link : links) {
                    auto name = link->GetFileName();
                    if (name.size() > max_length) {
                        // + 1 to give it some extra spacing.
                        max_length = name.size() + 1;
                    }
                }

                // Print the links and fix indentation.
                for (auto& link : links) {
                    auto name = link->GetFileName();
                    if (name.size() <= max_length) {
                        for (size_t i = name.size(); i <= max_length; i++) {
                            name += " ";
                        }
                    }
                    std::cout << name << " -> " << link->link_destination.string() << std::endl;
                }
            }
        }
    } else if (argv[1] == "rename"sv) {
        // Rename command allows a user to rename a link to have 2 seperate links
        // to 2 differnt executables, which are both named the same.
        if (argc < 3) {
            std::cout << "Please enter a old and new name. <loom rename old new>." << std::endl;
        } else {
            std::string oldName(argv[2]);
            std::string newName(argv[3]);

            Shortcut* sh = Shortcut::FromFile(oldName + ".cmd");
            sh->RenameLink(newName);
            std::cout << "Successfully renamed " << oldName << " to " << newName << "." << std::endl;
        }
    } else if (argv[1] == "init"sv) {
        if (AddLoomToPath()) {
            std::cout << "Loom has been initialized!" << std::endl;
        }
    } else {
        std::cout << "Unknown command. Use <loom add|rename|list|init> instead.";
    }

    return 0;
}
