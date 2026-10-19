#pragma once
#include <filesystem>
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

inline void CopyDirectoryToRelease(const std::string& srcDir) {
    std::error_code ec;
    if (!fs::exists(srcDir)) return;

    for (auto& entry : fs::recursive_directory_iterator(srcDir)) {
        if (!entry.is_regular_file()) continue;
        if (ShouldSkipAsset(entry.path())) continue;

        auto relPath = fs::relative(entry.path(), srcDir, ec);
        if (ec) { ec.clear(); continue; }

        auto targetPath = fs::path("release") / srcDir / relPath;

        fs::create_directories(targetPath.parent_path(), ec);
        ec.clear();

        fs::copy_file(entry.path(), targetPath,
                      fs::copy_options::overwrite_existing, ec);
        if (ec) {
            std::cout << "警告: 复制 " << entry.path() << " 失败 -> "
                      << ec.message() << "\n";
            ec.clear();
        }
    }
}

inline void BuildReleaseDir() {
    std::error_code ec;

    if (fs::exists("release")) fs::remove_all("release", ec);
    fs::create_directories("release", ec);
    if (ec) {
        std::cout << "警告: 无法创建 release/ -> " << ec.message() << "\n";
        return;
    }

    if (fs::exists("game.exe")) {
        fs::copy_file("game.exe", "release/game.exe",
                      fs::copy_options::overwrite_existing, ec);
        if (ec) {
            std::cout << "警告: 复制 game.exe 失败 -> " << ec.message() << "\n";
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