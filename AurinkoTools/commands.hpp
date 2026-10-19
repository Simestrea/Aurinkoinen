#pragma once
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include "utils.hpp"
#include "project.hpp"
#include "scan.hpp"
#include "codegen.hpp"
#include "packer.hpp"

namespace fs = std::filesystem;

// 角色脚本禁止引用的内部符号（都在 aur::detail 里）
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
// 命令：创建项目
// ==========================================
inline void CmdCreateProject() {
    if (fs::exists("project.txt")) {
        std::cout << "项目已存在（找到 project.txt）\n";
        return;
    }

    std::string wStr = Ask("画布宽度: ");
    std::string hStr = Ask("画布高度: ");
    int w = std::stoi(wStr);
    int h = std::stoi(hStr);

    WriteProject(w, h);
    fs::create_directories("characters");
    WriteGlobalsIfMissing();
    EnsureFontDir();
    EnsureBuildFile();

    std::cout << "\n项目创建完成\n";
    std::cout << "  画布尺寸: " << w << " x " << h << " (已保存到 project.txt)\n";
    std::cout << "  角色目录: characters/\n";
    std::cout << "  字体目录: fonts/\n";
    std::cout << "  编译配置: build.txt\n";
    std::cout << "  全局变量: globals.hpp\n";
}

// ==========================================
// 命令：创建角色
// ==========================================
inline void CmdCreateCharacter(const std::string& name) {
    EnsureProject();

    fs::path charDir = fs::path("characters") / name;
    if (fs::exists(charDir)) {
        std::cout << "错误: 角色 " << name << " 已经存在\n";
        return;
    }

    fs::create_directories(charDir);

    std::cout << "请输入精灵图路径（支持拖拽文件到终端，或直接输入完整路径）\n";
    std::string inputPath = StripQuotes(Ask("图片路径: "));

    fs::path sourceImage;
    if (inputPath.empty()) {
        std::cout << "未提供图片路径，跳过图片复制\n";
    } else {
        sourceImage = fs::path(inputPath);
        if (!fs::exists(sourceImage)) {
            std::cout << "警告: 文件不存在 -> " << sourceImage << "\n";
            std::cout << "继续创建角色，稍后你可以手动把图片放到 characters/" << name << "/ 下\n";
        }
    }

    std::string textureName = name + ".png";
    if (!sourceImage.empty() && sourceImage.has_extension()) {
        textureName = name + sourceImage.extension().string();
    }

    if (!sourceImage.empty() && fs::exists(sourceImage)) {
        fs::path targetImage = charDir / textureName;
        std::error_code ec;
        bool same = fs::exists(targetImage) &&
                    fs::equivalent(sourceImage, targetImage, ec);
        if (same) {
            std::cout << "图片已在目标位置，跳过复制\n";
        } else {
            fs::copy_file(sourceImage, targetImage, fs::copy_options::overwrite_existing, ec);
            if (ec) std::cout << "图片复制失败: " << ec.message() << "\n";
            else     std::cout << "图片已复制 -> characters/" << name << "/" << textureName << "\n";
        }
    }

    std::string scaleStr = Ask("缩放倍数 (默认 2.0): ");
    float scale = scaleStr.empty() ? 2.0f : std::stof(scaleStr);

    std::string framesStr = Ask("精灵图帧数 (默认 10): ");
    int frames = framesStr.empty() ? 10 : std::stoi(framesStr);

    std::string speedStr = Ask("每帧持续秒数 (默认 0.15): ");
    float frameSpeed = speedStr.empty() ? 0.15f : std::stof(speedStr);

    std::string sceneStr = Ask("场景编号 (默认 0): ");
    int scene = sceneStr.empty() ? 0 : std::stoi(sceneStr);

    {
        std::ofstream out(charDir / "config.txt");
        out << name << " " << textureName << " " << scale << " "
            << frames << " " << frameSpeed << " " << scene << "\n";
    }

    {
        std::ofstream out(charDir / (name + ".hpp"));
        out << "#pragma once\n";
        out << "#include <aurinko/character.hpp>\n";
        out << "#include <aurinko/coroutine.hpp>\n";
        out << "#include <aurinko/scene.hpp>\n";
        out << "#include <aurinko/text.hpp>\n";
        out << "#include <aurinko/context.hpp>\n";
        out << "#include \"globals.hpp\"\n";
        out << "\n";
        out << "inline aur::Task " << name << "Fun(aur::Character& c, aur::Context& ctx) {\n";
        out << "    // " << name << " 的剧本\n";
        out << "    //\n";
        out << "    // 参数说明（顺序自由、个数自由，只认类型）:\n";
        out << "    //   aur::Character&  -> 自动传入当前角色实例\n";
        out << "    //   aur::Context&    -> 自动传入上下文包，通过它访问全局资源：\n";
        out << "    //       ctx.scene      场景管理器\n";
        out << "    //       ctx.screen     虚拟屏幕（查尺寸、坐标换算）\n";
        out << "    //       ctx.fonts      字体池\n";
        out << "    //       ctx.textures   纹理池\n";
        out << "    //       ctx.scheduler  协程调度器（一般不用直接碰）\n";
        out << "    //\n";
        out << "    // 例：\n";
        out << "    //   auto* bob = ctx.scene.Find(\"Bob\");\n";
        out << "    //   ctx.scene.Switch(1);\n";
        out << "    //   int vw = ctx.screen.GetVirtualWidth();\n";
        out << "    //\n";
        out << "    // 注意：禁止直接访问 aur::detail:: 里的内部单例\n";
        out << "    //\n";
        out << "    // 示例:\n";
        out << "    //   c.Say(\"Hello!\", 1.0f);\n";
        out << "    //   co_await aur::WaitFor{ 1.0f };\n";
        out << "    co_return;\n";
        out << "}\n";
    }

    std::cout << "\n角色 " << name << " 创建完成\n";
    std::cout << "  目录:   characters/" << name << "/\n";
    std::cout << "  头文件: characters/" << name << "/" << name << ".hpp\n";
    std::cout << "  图片:   characters/" << name << "/" << textureName << "\n";
    std::cout << "  配置:   characters/" << name << "/config.txt\n";
    std::cout << "  场景:   " << scene << "\n";
}

// ==========================================
// 命令：导入字体
// ==========================================
inline void CmdAddFont(const std::string& inputPath, const std::string& newName) {
    EnsureProject();

    fs::path source = fs::path(StripQuotes(inputPath));
    if (!fs::exists(source)) {
        std::cout << "错误: 文件不存在 -> " << source << "\n";
        return;
    }
    if (!fs::is_regular_file(source)) {
        std::cout << "错误: 不是普通文件 -> " << source << "\n";
        return;
    }

    auto ext = source.extension().string();
    std::string extLower = ToLower(ext);

    if (extLower != ".ttf" && extLower != ".ttc" && extLower != ".otf") {
        std::cout << "警告: " << ext << " 不是常见字体格式（.ttf / .ttc / .otf）\n";
        std::cout << "是否继续？(y/N): ";
        std::string ans;
        std::getline(std::cin, ans);
        ans = Trim(ans);
        if (ans != "y" && ans != "Y") {
            std::cout << "已取消\n";
            return;
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
        std::cout << "字体已在目标位置，跳过复制\n";
        return;
    }

    bool overwrite = false;
    if (fs::exists(target) && !same) {
        std::cout << "fonts/" << targetName << " 已存在，是否覆盖？(y/N): ";
        std::string ans;
        std::getline(std::cin, ans);
        ans = Trim(ans);
        if (ans != "y" && ans != "Y") {
            std::cout << "已取消\n";
            return;
        }
        overwrite = true;
    }

    auto opts = overwrite ? fs::copy_options::overwrite_existing
                          : fs::copy_options::none;

    fs::copy_file(source, target, opts, ec);
    if (ec) {
        std::cout << "字体复制失败: " << ec.message() << "\n";
        return;
    }

    std::cout << "\n字体已导入\n";
    std::cout << "  文件: fonts/" << targetName << "\n";
    std::cout << "  使用:\n";
    std::cout << "    aur::Text label(\"Hello\", 10, 10, 20, DARKGRAY,\n";
    std::cout << "                    \"fonts/" << targetName << "\");\n";
    std::cout << "\n";
    std::cout << "  下次 aurinko update 会自动打包进 release/fonts/\n";
}

// ==========================================
// 命令：更新
// ==========================================
inline void CmdUpdate(bool debugMode, const std::vector<std::string>& extraFlags) {
    EnsureProject();

    int width = 0, height = 0;
    ReadProject(width, height);

    auto chars = ReadAllCharacters();
    if (chars.empty()) {
        std::cout << "提示: 当前没有角色，请先执行 aurinko create <角色名>\n";
    }

    // ---------- 扫描角色 hpp，拒绝内部单例 ----------
    {
        const auto& forbidden = ForbiddenSymbols();
        bool hasViolation = false;

        for (auto& c : chars) {
            fs::path hppPath = fs::path("characters") / c.name / (c.name + ".hpp");
            auto violations = CheckForbidden(hppPath, forbidden);

            for (auto& v : violations) {
                std::cout << "错误: " << v.file << ":" << v.line
                          << " 使用了被禁止的内部符号\n";
                std::cout << "  代码: " << v.text << "\n";
                hasViolation = true;
            }
        }

        if (hasViolation) {
            std::cout << "\n角色脚本禁止直接访问内部单例。\n";
            std::cout << "请在函数签名里加 aur::SceneManager& scene 参数，用 scene 访问场景。\n";
            return;
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
        std::cout << "错误: 生成 index.cpp 失败\n";
        return;
    }

    std::cout << "已生成 index.cpp (" << chars.size() << " 个角色, "
              << sceneList.size() << " 个场景)\n";
    std::cout << "正在编译...\n";

    std::string cmd = "g++ index.cpp -o game -I. "
                      "-mwindows "
                      "-lraylib -lwinmm -lgdi32 -luser32 "
                      "-std=c++20";

    if (debugMode) cmd += " -g -O0";
    else           cmd += " -O2 -s";

    for (auto& f : extraFlags) { cmd += " "; cmd += f; }

    std::cout << "  " << cmd << "\n";

    const int result = std::system(cmd.c_str());
    if (result == 0) {
        std::cout << "编译成功 -> game.exe\n";
        if (!debugMode) {
            std::cout << "正在生成 release/ ...\n";
            BuildReleaseDir();
            std::cout << "release/ 已生成\n";
        } else {
            std::cout << "(debug 模式，跳过 release/)\n";
        }
    } else {
        std::cout << "编译失败 (退出码 " << result << ")\n";
    }
}