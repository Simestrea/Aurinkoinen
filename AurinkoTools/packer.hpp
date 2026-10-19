#pragma once
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include "utils.hpp"

namespace fs = std::filesystem;

inline bool ShouldSkipAsset(const fs::path& p) {
    auto ext = ToLower(p.extension().string());
    if (ext == ".hpp" || ext == ".h" || ext == ".cpp" || ext == ".c") return true;
    if (ext == ".txt") return true;
    if (ext == ".obj" || ext == ".o" || ext == ".tmp") return true;
    if (ext == ".exe") return true;
    return false;
}

// Read characters/<name>/config.txt to check whether it is disabled (scale < 0).
// If unreadable, treat as enabled.
inline bool IsCharacterDisabled(const std::string& charName) {
    fs::path cfg = fs::path("characters") / charName / "config.txt";
    if (!fs::exists(cfg)) return false;

    std::ifstream in(cfg);
    if (!in.is_open()) return false;

    std::string n, tex;
    float sc;
    int fr, sn;
    float fs_;
    if (in >> n >> tex >> sc >> fr >> fs_ >> sn) {
        return sc < 0.0f;
    }
    return false;
}

inline void CopyDirectoryToRelease(const std::string& srcDir) {
    std::error_code ec;
    if (!fs::exists(srcDir)) return;

    for (auto& entry : fs::recursive_directory_iterator(srcDir)) {
        if (!entry.is_regular_file()) continue;
        if (ShouldSkipAsset(entry.path())) continue;

        auto relPath = fs::relative(entry.path(), srcDir, ec);
        if (ec) { ec.clear(); continue; }

        // If under characters/, skip assets of disabled characters
        if (srcDir == "characters" && !relPath.empty()) {
            std::string charName = relPath.begin()->string();
            if (IsCharacterDisabled(charName)) continue;
        }

        auto targetPath = fs::path("release") / srcDir / relPath;

        fs::create_directories(targetPath.parent_path(), ec);
        ec.clear();

        fs::copy_file(entry.path(), targetPath,
                      fs::copy_options::overwrite_existing, ec);
        if (ec) {
            std::cout << "Warning: failed to copy " << entry.path()
                      << " -> " << ec.message() << "\n";
            ec.clear();
        }
    }
}

inline void BuildReleaseDir() {
    std::error_code ec;

    if (fs::exists("release")) fs::remove_all("release", ec);
    fs::create_directories("release", ec);
    if (ec) {
        std::cout << "Warning: cannot create release/ -> " << ec.message() << "\n";
        return;
    }

    if (fs::exists("game.exe")) {
        fs::copy_file("game.exe", "release/game.exe",
                      fs::copy_options::overwrite_existing, ec);
        if (ec) {
            std::cout << "Warning: failed to copy game.exe -> " << ec.message() << "\n";
            ec.clear();
        }
    }

    CopyDirectoryToRelease("characters");

    if (fs::exists("fonts")) {
        fs::create_directories("release/fonts", ec);
        ec.clear();
        CopyDirectoryToRelease("fonts");
    }
}