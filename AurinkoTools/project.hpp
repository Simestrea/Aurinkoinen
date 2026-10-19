#pragma once
#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>
#include <string>
#include <vector>
#include "utils.hpp"

namespace fs = std::filesystem;

struct CharacterConfig {
    std::string name;
    std::string texture;
    float scale;
    int frames;
    float frameSpeed;
    int scene;

    // Convention: scale < 0 -> disabled, skip in index.cpp
    bool IsDisabled() const { return scale < 0.0f; }

    // Convention: texture "-" or empty -> logic-only character
    bool HasTexture() const {
        return !texture.empty() && texture != "-";
    }
};

// ---------- project.txt ----------
inline bool ReadProject(int& width, int& height) {
    std::ifstream in("project.txt");
    if (!in.is_open()) return false;
    in >> width >> height;
    return true;
}

inline void WriteProject(int width, int height) {
    std::ofstream out("project.txt");
    out << width << " " << height << "\n";
}

// ---------- globals.hpp ----------
inline void WriteGlobalsIfMissing() {
    if (fs::exists("globals.hpp")) return;
    std::ofstream out("globals.hpp");
    out << "#pragma once\n\n";
    out << "namespace Globals {\n";
    out << "    inline float timer = 0.0f;\n";
    out << "}\n";
}

// ---------- styles/ ----------
inline void EnsureStylesDir() {
    if (fs::exists("styles")) return;
    fs::create_directories("styles");
}

// Write an empty .style file. Returns false if it already exists.
inline bool WriteStyleFile(const std::string& name) {
    fs::path target = fs::path("styles") / (name + ".style");
    if (fs::exists(target)) return false;

    std::ofstream out(target, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) return false;
    out << "# Stylesheet: " << name << "\n";
    out << "# Edit this file, then call Globals::styles.Load(\"styles/"
        << name << ".style\");\n";
    out << "\n";
    return true;
}

// ---------- fonts/ ----------
inline void EnsureFontDir() {
    if (fs::exists("fonts")) return;
    fs::create_directories("fonts");

    std::ofstream out("fonts/README.txt");
    out << "Put your .ttf / .ttc font files here.\n";
    out << "Example:\n";
    out << "  fonts/msyh.ttc\n";
    out << "  fonts/impact.ttf\n\n";
    out << "Use in a character script:\n";
    out << "  aur::Text label(\"Hello\", 10, 10, 20, DARKGRAY, \"fonts/impact.ttf\");\n";
}

// ---------- build.txt ----------
inline void EnsureBuildFile() {
    if (fs::exists("build.txt")) return;
    std::ofstream out("build.txt");
    out << "# Extra compile flags, one per line\n";
    out << "# aurinko update reads and appends them automatically\n";
    out << "#\n";
    out << "# Examples:\n";
    out << "#   -lcurl\n";
    out << "#   -lpthread\n";
    out << "#   -I./thirdparty\n";
    out << "#   -DDEBUG_MODE\n";
    out << "#\n";
    out << "# Lines starting with # are ignored\n";
}

inline std::vector<std::string> ReadBuildFlags() {
    std::vector<std::string> flags;
    std::ifstream in("build.txt");
    if (!in.is_open()) return flags;

    std::string line;
    while (std::getline(in, line)) {
        auto hashPos = line.find('#');
        if (hashPos != std::string::npos) line = line.substr(0, hashPos);
        line = Trim(line);
        if (line.empty()) continue;
        std::istringstream iss(line);
        std::string tok;
        while (iss >> tok) flags.push_back(tok);
    }
    return flags;
}

// ---------- Project init ----------
inline void EnsureProject() {
    if (!fs::exists("project.txt")) {
        std::cout << "project.txt not found, creating default project (800x450)\n\n";
        WriteProject(800, 450);
        fs::create_directories("characters");
    }
    WriteGlobalsIfMissing();
    EnsureFontDir();
    EnsureStylesDir();
    EnsureBuildFile();
}

// ---------- Character list ----------
inline std::vector<CharacterConfig> ReadAllCharacters(bool skipDisabled = true) {
    std::vector<CharacterConfig> result;
    if (!fs::exists("characters")) return result;

    for (auto& entry : fs::directory_iterator("characters")) {
        if (!entry.is_directory()) continue;

        auto configPath = entry.path() / "config.txt";
        if (!fs::exists(configPath)) continue;

        std::ifstream in(configPath);
        CharacterConfig cfg;
        if (in >> cfg.name >> cfg.texture >> cfg.scale
               >> cfg.frames >> cfg.frameSpeed >> cfg.scene) {
            if (skipDisabled && cfg.IsDisabled()) continue;
            result.push_back(cfg);
        }
    }
    return result;
}