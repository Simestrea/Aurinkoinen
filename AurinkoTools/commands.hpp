#pragma once
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cctype>
#include "utils.hpp"
#include "project.hpp"
#include "scan.hpp"
#include "codegen.hpp"
#include "packer.hpp"

namespace fs = std::filesystem;

// Symbols forbidden inside character scripts (all under aur::detail)
inline const std::vector<std::string>& ForbiddenSymbols() {
    static std::vector<std::string> list = {
        "detail::context",
        "detail::sceneManager",
        "detail::scheduler",
        "detail::virtualScreen",
        "detail::texturePool",
        "detail::fontPool"
    };
    return list;
}

// ==========================================
// Command: create project
//   返回: 0 = 成功, 1 = 失败
// ==========================================
inline int CmdCreateProject() {
    if (fs::exists("project.txt")) {
        std::cout << "Project already exists (found project.txt)\n";
        return 1;
    }

    std::string wStr = Ask("Canvas width: ");
    std::string hStr = Ask("Canvas height: ");

    int w = 0, h = 0;
    try {
        w = std::stoi(wStr);
        h = std::stoi(hStr);
    } catch (...) {
        std::cout << "Error: width/height must be integers\n";
        return 1;
    }

    if (w <= 0 || h <= 0) {
        std::cout << "Error: width/height must be positive\n";
        return 1;
    }

    WriteProject(w, h);
    fs::create_directories("characters");
    WriteGlobalsIfMissing();
    EnsureFontDir();
    EnsureBuildFile();

    std::cout << "\nProject created\n";
    std::cout << "  Canvas size:   " << w << " x " << h << " (saved to project.txt)\n";
    std::cout << "  Characters:    characters/\n";
    std::cout << "  Fonts:         fonts/\n";
    std::cout << "  Build flags:   build.txt\n";
    std::cout << "  Globals:       globals.hpp\n";
    return 0;
}

// ==========================================
// Command: create character
//   返回: 0 = 成功, 1 = 失败
// ==========================================
inline int CmdCreateCharacter(const std::string& name) {
    EnsureProject();

    fs::path charDir = fs::path("characters") / name;
    if (fs::exists(charDir)) {
        std::cout << "Error: character \"" << name << "\" already exists\n";
        return 1;
    }

    if (name.empty()) {
        std::cout << "Error: character name cannot be empty\n";
        return 1;
    }

    fs::create_directories(charDir);

    std::cout << "Enter sprite sheet path\n";
    std::cout << "  empty = logic-only character (coroutine / UI controller)\n";
    std::cout << "  drag-and-drop a file onto the terminal is supported\n";
    std::string inputPath = StripQuotes(Ask("Image path: "));

    std::string textureName = "-";     // default: no texture
    bool hasTexture = false;

    if (!inputPath.empty()) {
        fs::path sourceImage = fs::path(inputPath);
        if (!fs::exists(sourceImage)) {
            std::cout << "Warning: file not found -> " << sourceImage << "\n";
            std::cout << "Continuing; you can copy the image into characters/"
                      << name << "/ later\n";
        }
        textureName = name + (sourceImage.has_extension()
                              ? sourceImage.extension().string()
                              : ".png");
        hasTexture = true;

        if (fs::exists(sourceImage)) {
            fs::path targetImage = charDir / textureName;
            std::error_code ec;
            bool same = fs::exists(targetImage) &&
                        fs::equivalent(sourceImage, targetImage, ec);
            if (same) {
                std::cout << "Image already at target, skipping copy\n";
            } else {
                fs::copy_file(sourceImage, targetImage,
                              fs::copy_options::overwrite_existing, ec);
                if (ec) std::cout << "Image copy failed: " << ec.message() << "\n";
                else     std::cout << "Image copied -> characters/"
                                   << name << "/" << textureName << "\n";
            }
        }
    } else {
        std::cout << "No image provided, creating logic-only character\n";
    }

    float scale      = 1.0f;
    int   frames     = 1;
    float frameSpeed = 0.15f;

    if (hasTexture) {
        std::string scaleStr = Ask("Scale (default 2.0): ");
        try { scale = scaleStr.empty() ? 2.0f : std::stof(scaleStr); }
        catch (...) { scale = 2.0f; }

        std::string framesStr = Ask("Sprite frame count (default 10): ");
        try { frames = framesStr.empty() ? 10 : std::stoi(framesStr); }
        catch (...) { frames = 10; }

        std::string speedStr = Ask("Seconds per frame (default 0.15): ");
        try { frameSpeed = speedStr.empty() ? 0.15f : std::stof(speedStr); }
        catch (...) { frameSpeed = 0.15f; }
    }

    std::string sceneStr = Ask("Scene id (default 0): ");
    int scene = 0;
    try { scene = sceneStr.empty() ? 0 : std::stoi(sceneStr); }
    catch (...) { scene = 0; }

    {
        std::ofstream out(charDir / "config.txt");
        out << name << " " << textureName << " " << scale << " "
            << frames << " " << frameSpeed << " " << scene << "\n";
    }

    {
        std::ofstream out(charDir / (name + ".hpp"));
        out << "#pragma once\n";
        out << "#include <aurinko/aurinko.hpp>\n";
        out << "#include \"globals.hpp\"\n";
        out << "\n";
        out << "inline aur::Task " << name
            << "Fun([[maybe_unused]] aur::Character& c, "
            << "[[maybe_unused]] aur::Context& ctx) {\n";
        out << "    // Script for " << name << "\n";
        out << "    //\n";
        out << "    // Parameters (order/free, only types matter):\n";
        out << "    //   aur::Character&  -> injected current character\n";
        out << "    //   aur::Context&    -> injected context bundle:\n";
        out << "    //       ctx.scene      scene manager\n";
        out << "    //       ctx.screen     virtual screen (size, coord conversion)\n";
        out << "    //       ctx.fonts      font pool\n";
        out << "    //       ctx.textures   texture pool\n";
        out << "    //       ctx.scheduler  coroutine scheduler\n";
        out << "    //\n";
        out << "    // Example:\n";
        out << "    //   auto* bob = ctx.scene.Find(\"Bob\");\n";
        out << "    //   ctx.scene.Switch(1);\n";
        out << "    //   int vw = ctx.screen.GetVirtualWidth();\n";
        out << "    //\n";
        out << "    // Note: do NOT access aur::detail::* directly\n";
        out << "    //\n";
        out << "    //   c.Say(\"Hello!\", 1.0f);\n";
        out << "    //   co_await aur::WaitFor{ 1.0f };\n";
        out << "    co_return;\n";
        out << "}\n";
    }

    std::cout << "\nCharacter \"" << name << "\" created\n";
    std::cout << "  Directory: characters/" << name << "/\n";
    std::cout << "  Header:    characters/" << name << "/" << name << ".hpp\n";
    if (hasTexture) {
        std::cout << "  Image:     characters/" << name << "/" << textureName << "\n";
    } else {
        std::cout << "  Image:     (none, logic-only)\n";
    }
    std::cout << "  Config:    characters/" << name << "/config.txt\n";
    std::cout << "  Scene:     " << scene << "\n";
    std::cout << "\n";
    std::cout << "  Tip: set the scale field in characters/" << name << "/config.txt\n";
    std::cout << "       to -1 to disable this character; it will be skipped on update.\n";
    return 0;
}

// ==========================================
// Command: import font
//   返回: 0 = 成功, 1 = 失败/取消
// ==========================================
inline int CmdAddFont(const std::string& inputPath, const std::string& newName) {
    EnsureProject();

    fs::path source = fs::path(StripQuotes(inputPath));
    if (!fs::exists(source)) {
        std::cout << "Error: file not found -> " << source << "\n";
        return 1;
    }
    if (!fs::is_regular_file(source)) {
        std::cout << "Error: not a regular file -> " << source << "\n";
        return 1;
    }

    auto ext = source.extension().string();
    std::string extLower = ToLower(ext);

    if (extLower != ".ttf" && extLower != ".ttc" && extLower != ".otf") {
        std::cout << "Warning: " << ext << " is not a common font format (.ttf / .ttc / .otf)\n";
        std::cout << "Continue? (y/N): ";
        std::string ans;
        std::getline(std::cin, ans);
        ans = Trim(ans);
        if (ans != "y" && ans != "Y") {
            std::cout << "Cancelled\n";
            return 1;
        }
    }

    std::string targetName;
    if (!newName.empty()) {
        targetName = newName;
        if (fs::path(targetName).extension().empty()) targetName += ext;
    } else {
        targetName = source.filename().string();
    }

    fs::path target = fs::path("fonts") / targetName;

    std::error_code ec;
    bool same = fs::exists(target) && fs::equivalent(source, target, ec);
    if (same) {
        std::cout << "Font already at target, skipping copy\n";
        return 0;
    }

    bool overwrite = false;
    if (fs::exists(target) && !same) {
        std::cout << "fonts/" << targetName << " already exists. Overwrite? (y/N): ";
        std::string ans;
        std::getline(std::cin, ans);
        ans = Trim(ans);
        if (ans != "y" && ans != "Y") {
            std::cout << "Cancelled\n";
            return 1;
        }
        overwrite = true;
    }

    auto opts = overwrite ? fs::copy_options::overwrite_existing
                          : fs::copy_options::none;

    fs::copy_file(source, target, opts, ec);
    if (ec) {
        std::cout << "Font copy failed: " << ec.message() << "\n";
        return 1;
    }

    std::cout << "\nFont imported\n";
    std::cout << "  File: fonts/" << targetName << "\n";
    std::cout << "  Usage:\n";
    std::cout << "    aur::Text label(\"Hello\", 10, 10, 20, DARKGRAY,\n";
    std::cout << "                    \"fonts/" << targetName << "\");\n";
    std::cout << "\n";
    std::cout << "  Next aurinko update will pack it into release/fonts/\n";
    return 0;
}


// ==========================================
// Command: delete character
//   返回: 0 = 成功, 1 = 失败/取消
// ==========================================
inline int CmdDeleteCharacter(const std::string& name) {
    EnsureProject();

    if (name.empty()) {
        std::cout << "Usage: aurinko delete <name>\n";
        return 1;
    }

    fs::path charDir = fs::path("characters") / name;
    if (!fs::exists(charDir)) {
        std::cout << "Error: character \"" << name << "\" not found\n";
        return 1;
    }
    if (!fs::is_directory(charDir)) {
        std::cout << "Error: not a directory -> " << charDir << "\n";
        return 1;
    }

    std::vector<std::string> files;
    std::error_code ec;
    for (auto& e : fs::recursive_directory_iterator(charDir, ec)) {
        if (e.is_regular_file()) {
            auto rel = fs::relative(e.path(), charDir, ec);
            files.push_back(rel.string());
        }
    }

    std::cout << "Delete character \"" << name << "\"?\n";
    std::cout << "  Directory: " << charDir.string() << "\n";
    std::cout << "  Files (" << files.size() << "):\n";
    for (auto& f : files) std::cout << "    " << f << "\n";
    std::cout << "Confirm? (y/N): ";

    std::string ans;
    std::getline(std::cin, ans);
    ans = Trim(ans);
    if (ans != "y" && ans != "Y") {
        std::cout << "Cancelled\n";
        return 1;
    }

    std::uintmax_t n = fs::remove_all(charDir, ec);
    if (ec) {
        std::cout << "Error: remove failed -> " << ec.message() << "\n";
        return 1;
    }

    std::cout << "\nCharacter \"" << name << "\" deleted\n";
    std::cout << "  Removed " << n << " entries\n";
    std::cout << "  Next aurinko update regenerates index.cpp without it\n";
    return 0;
}

// ==========================================
// Command: list characters
//   返回: 0 = 成功
// ==========================================
inline int CmdListCharacters() {
    EnsureProject();

    auto chars = ReadAllCharacters(/*skipDisabled=*/false);
    if (chars.empty()) {
        std::cout << "No characters found in characters/\n";
        return 0;
    }

    std::cout << "\n";
    std::cout << "Characters (" << chars.size() << "):\n";
    std::cout << "  " << std::string(72, '-') << "\n";
    std::printf("  %-20s %-20s %8s %6s %8s %6s\n",
                "name", "texture", "scale", "frames", "speed", "scene");
    std::cout << "  " << std::string(72, '-') << "\n";
    for (auto& c : chars) {
        std::printf("  %-20s %-20s %8.2f %6d %8.3f %6d%s\n",
                    c.name.c_str(),
                    c.texture.empty() ? "-" : c.texture.c_str(),
                    c.scale, c.frames, c.frameSpeed, c.scene,
                    c.IsDisabled() ? "  [disabled]" : "");
    }
    std::cout << "\n";
    return 0;
}

// ==========================================
// Command: create stylesheet (empty)
//   返回: 0 = 成功, 1 = 失败
// ==========================================
inline int CmdCreateStyle(const std::string& name) {
    EnsureProject();
    EnsureStylesDir();

    if (name.empty()) {
        std::cout << "Usage: aurinko create-style <name>\n";
        return 1;
    }

    for (char c : name) {
        if (!std::isalnum(static_cast<unsigned char>(c)) &&
            c != '_' && c != '-') {
            std::cout << "Error: name may only contain letters, digits, _ or -\n";
            return 1;
        }
    }

    if (!WriteStyleFile(name)) {
        std::cout << "Error: styles/" << name
                  << ".style already exists, or cannot write\n";
        return 1;
    }

    std::cout << "\nStylesheet created (empty)\n";
    std::cout << "  File: styles/" << name << ".style\n";
    std::cout << "\n";
    std::cout << "  Usage:\n";
    std::cout << "    Globals::styles.Load(\"styles/" << name << ".style\");\n";
    std::cout << "    Globals::styles.Apply(btn, \"button primary\");\n";
    return 0;
}


// ============================================================
// 找 g++ 可执行文件
//   优先级：
//     1) 本 exe 所在目录的 g++.exe / g++ / gcc.exe / gcc
//     2) 直接返回 "g++.exe"（靠 PATH）
// ============================================================
#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#endif

inline std::string FindGppExecutable() {
#ifdef _WIN32
    char buf[MAX_PATH];
    DWORD n = GetModuleFileNameA(NULL, buf, MAX_PATH);
    if (n > 0 && n < MAX_PATH) {
        std::filesystem::path selfPath(std::string(buf, n));
        std::filesystem::path dir = selfPath.parent_path();
        for (auto name : { "g++.exe", "g++", "gcc.exe", "gcc" }) {
            std::filesystem::path candidate = dir / name;
            std::error_code ec;
            if (std::filesystem::exists(candidate, ec)
                && std::filesystem::is_regular_file(candidate, ec)) {
                return "\"" + candidate.string() + "\"";
            }
        }
    }
#endif
    return "g++.exe";
}

// ==========================================
// Command: update
//   返回: 0 = 编译成功, 1 = 失败
// ==========================================
inline int CmdUpdate(bool debugMode, const std::vector<std::string>& extraFlags) {
    EnsureProject();

    int width = 0, height = 0;
    ReadProject(width, height);

    auto allChars = ReadAllCharacters(/*skipDisabled=*/false);
    auto chars    = ReadAllCharacters(/*skipDisabled=*/true);

    // ---------- Report disabled characters ----------
    {
        std::vector<std::string> disabled;
        for (auto& c : allChars) {
            if (c.IsDisabled()) disabled.push_back(c.name);
        }
        if (!disabled.empty()) {
            std::cout << "Disabled (scale < 0, skipped in index.cpp): ";
            for (size_t i = 0; i < disabled.size(); ++i) {
                if (i > 0) std::cout << ", ";
                std::cout << disabled[i];
            }
            std::cout << "\n";
        }
    }

    // ---------- Report no-texture characters ----------
    {
        std::vector<std::string> noTex;
        for (auto& c : chars) {
            if (!c.HasTexture()) noTex.push_back(c.name);
        }
        if (!noTex.empty()) {
            std::cout << "No-texture (texture = \"-\"): ";
            for (size_t i = 0; i < noTex.size(); ++i) {
                if (i > 0) std::cout << ", ";
                std::cout << noTex[i];
            }
            std::cout << "\n";
        }
    }

    if (chars.empty()) {
        std::cout << "Note: no active characters, run aurinko create <name> first\n";
    }

    // ---------- Scan character hpp, reject internal singletons ----------
    {
        const auto& forbidden = ForbiddenSymbols();
        bool hasViolation = false;

        for (auto& c : chars) {
            fs::path hppPath = fs::path("characters") / c.name / (c.name + ".hpp");
            auto violations = CheckForbidden(hppPath, forbidden);

            for (auto& v : violations) {
                std::cout << "Error: " << v.file << ":" << v.line
                          << " uses a forbidden internal symbol\n";
                std::cout << "  Code: " << v.text << "\n";
                hasViolation = true;
            }
        }

        if (hasViolation) {
            std::cout << "\nCharacter scripts must not access internal singletons.\n";
            std::cout << "Add an aur::Context& ctx parameter and use it instead:\n";
            std::cout << "  ctx.scene      scene manager\n";
            std::cout << "  ctx.screen     virtual screen\n";
            std::cout << "  ctx.fonts      font pool\n";
            std::cout << "  ctx.textures   texture pool\n";
            std::cout << "  ctx.scheduler  coroutine scheduler\n";
            return 1;
        }
    }

    std::vector<CharFunctions> charFuncs;
    for (auto& c : chars) {
        fs::path hppPath = fs::path("characters") / c.name / (c.name + ".hpp");
        auto funcs = DetectFunctions(hppPath);
        charFuncs.push_back({ c.name, funcs });
    }

    std::vector<int> sceneList;
    for (auto& c : chars) {
        if (std::find(sceneList.begin(), sceneList.end(), c.scene) == sceneList.end()) {
            sceneList.push_back(c.scene);
        }
    }
    std::sort(sceneList.begin(), sceneList.end());

    if (!GenerateIndexCpp(width, height, chars, charFuncs, sceneList)) {
        std::cout << "Error: failed to generate index.cpp\n";
        return 1;
    }

    std::cout << "Generated index.cpp (" << chars.size() << " characters, "
              << sceneList.size() << " scenes)\n";
    std::cout << "Compiling...\n";

    // 从本 exe 同目录找 g++，无需 PATH
    const std::string gpp = FindGppExecutable();
    std::string cmd = gpp + " index.cpp -o game -I. "
                      "-mwindows "
                      "-lraylib -lwinmm -lgdi32 -luser32 "
                      "-std=c++20";

    if (debugMode) cmd += " -g -O0";
    else           cmd += " -O2 -s";

    for (auto& f : extraFlags) { cmd += " "; cmd += f; }

    std::cout << "  " << cmd << "\n";

    const int result = std::system(cmd.c_str());

    if (result == 0) {
        std::cout << "Build succeeded -> game.exe\n";
        if (!debugMode) {
            std::cout << "Building release/ ...\n";
            BuildReleaseDir();
            std::cout << "release/ done\n";
        } else {
            std::cout << "(debug mode, skipping release/)\n";
        }
        return 0;
    } else {
        std::cout << "Build failed (exit code " << result << ")\n";
        return 1;
    }
}