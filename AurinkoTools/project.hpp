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
    out << "# 额外的编译参数，每行一条\n";
    out << "# aurinko update 时会自动读取并拼接\n";
    out << "#\n";
    out << "# 例如:\n";
    out << "#   -lcurl\n";
    out << "#   -lpthread\n";
    out << "#   -I./thirdparty\n";
    out << "#   -DDEBUG_MODE\n";
    out << "#\n";
    out << "# 以 # 开头的行会被忽略\n";
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

// ---------- 项目初始化 ----------
inline void EnsureProject() {
    if (!fs::exists("project.txt")) {
        std::cout << "未找到 project.txt，自动创建默认项目 (800x450)\n\n";
        WriteProject(800, 450);
        fs::create_directories("characters");
    }
    WriteGlobalsIfMissing();
    EnsureFontDir();
    EnsureBuildFile();
}

// ---------- 角色列表 ----------
inline std::vector<CharacterConfig> ReadAllCharacters() {
    std::vector<CharacterConfig> result;
    if (!fs::exists("characters")) return result;

    for (auto& entry : fs::directory_iterator("characters")) {
        if (!entry.is_directory()) continue;

        auto configPath = entry.path() / "config.txt";
        if (!fs::exists(configPath)) continue;

        std::ifstream in(configPath);
        CharacterConfig cfg;
        if (in >> cfg.name >> cfg.texture >> cfg.scale >> cfg.frames >> cfg.frameSpeed >> cfg.scene) {
            result.push_back(cfg);
        }
    }
    return result;
}