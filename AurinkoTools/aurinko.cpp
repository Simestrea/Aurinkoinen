#include <iostream>
#include <string>
#include <vector>
#include "version.hpp"
#include "utils.hpp"
#include "commands.hpp"

void PrintUsage() {
    std::cout << "\n";
    std::cout << "Aurinko - game project scaffolding\n";
    std::cout << "\n";
    std::cout << "Usage:\n";
    std::cout << "  aurinko create                  Create a new project (characters/ + fonts/)\n";
    std::cout << "  aurinko create <name>           Create a new character\n";
    std::cout << "  aurinko create-style <name>     Create an empty stylesheet\n";
    std::cout << "  aurinko delete <name>           Delete a character (with confirmation)\n";
    std::cout << "  aurinko list                    List all characters\n";
    std::cout << "  aurinko addfont <path> [name]   Import a font into fonts/\n";
    std::cout << "  aurinko update [args...]        Generate and compile (extra flags passed through)\n";
    std::cout << "  aurinko version                 Show version\n";
    std::cout << "\n";
    std::cout << "Extra update flags (examples):\n";
    std::cout << "  aurinko update -lcurl -lpthread   Add link libraries\n";
    std::cout << "  aurinko update -I./thirdparty     Add include path\n";
    std::cout << "  aurinko update -DDEBUG_MODE       Add macro definition\n";
    std::cout << "  aurinko update -d                 Debug build (-g -O0, no strip)\n";
    std::cout << "\n";
    std::cout << "You can also put one flag per line in build.txt (auto-read on update).\n";
    std::cout << "\n";
}

int main(int argc, char** argv) {
    if (argc < 2) {
        PrintUsage();
        return 1;
    }

    const std::string cmd = argv[1];

    if (cmd == "create") {
        if (argc >= 3) return CmdCreateCharacter(argv[2]);
        else           return CmdCreateProject();
    } else if (cmd == "create-style") {
        if (argc < 3) {
            std::cout << "Usage: aurinko create-style <name>\n";
            return 1;
        }
        return CmdCreateStyle(argv[2]);
    } else if (cmd == "delete" || cmd == "remove" || cmd == "rm") {
        if (argc < 3) {
            std::cout << "Usage: aurinko delete <name>\n";
            return 1;
        }
        return CmdDeleteCharacter(argv[2]);
    } else if (cmd == "list" || cmd == "ls") {
        return CmdListCharacters();
    } else if (cmd == "addfont") {
        if (argc < 3) {
            std::cout << "Usage: aurinko addfont <path> [name]\n";
            std::cout << "  <path>  font file path (drag-and-drop supported)\n";
            std::cout << "  [name]  optional, rename on import (extension auto-added)\n";
            return 1;
        }
        std::string inputPath = argv[2];
        std::string newName = (argc >= 4) ? argv[3] : "";
        return CmdAddFont(inputPath, newName);
    } else if (cmd == "update") {
        bool debugMode = false;
        std::vector<std::string> extraFlags;

        for (int i = 2; i < argc; i++) {
            std::string a = argv[i];
            if (a == "-d" || a == "--debug") debugMode = true;
            else                             extraFlags.push_back(a);
        }

        auto fileFlags = ReadBuildFlags();
        extraFlags.insert(extraFlags.end(), fileFlags.begin(), fileFlags.end());

        return CmdUpdate(debugMode, extraFlags);
    } else if (cmd == "version" || cmd == "-v" || cmd == "--version") {
        std::cout << "aurinko " << AURINKO_VERSION << "\n";
        return 0;
    } else {
        PrintUsage();
        return 1;
    }
}