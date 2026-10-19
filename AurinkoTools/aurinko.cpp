#include <iostream>
#include <string>
#include <vector>
#include "version.hpp"
#include "utils.hpp"
#include "commands.hpp"

void PrintUsage() {
    std::cout << "\n";
    std::cout << "Aurinko - 游戏项目脚手架\n";
    std::cout << "\n";
    std::cout << "用法:\n";
    std::cout << "  aurinko create                  创建新项目（生成 characters/ 和 fonts/）\n";
    std::cout << "  aurinko create <角色名>         创建新角色\n";
    std::cout << "  aurinko addfont <路径> [名字]   导入字体到 fonts/ 目录\n";
    std::cout << "  aurinko update [参数...]        生成并编译（可跟额外编译参数）\n";
    std::cout << "  aurinko version                 显示版本号\n";
    std::cout << "\n";
    std::cout << "update 额外参数例子:\n";
    std::cout << "  aurinko update -lcurl -lpthread   追加链接库\n";
    std::cout << "  aurinko update -I./thirdparty     追加 include 路径\n";
    std::cout << "  aurinko update -DDEBUG_MODE       追加宏定义\n";
    std::cout << "  aurinko update -d                 切到 debug 编译（-g -O0，不去符号）\n";
    std::cout << "\n";
    std::cout << "也可以在项目根目录创建 build.txt，每行一条参数，update 时自动读取。\n";
    std::cout << "\n";
}

int main(int argc, char** argv) {
    if (argc < 2) {
        PrintUsage();
        return 0;
    }

    const std::string cmd = argv[1];

    if (cmd == "create") {
        if (argc >= 3) CmdCreateCharacter(argv[2]);
        else           CmdCreateProject();
    } else if (cmd == "addfont") {
        if (argc < 3) {
            std::cout << "用法: aurinko addfont <路径> [名字]\n";
            std::cout << "  <路径>  字体文件路径（支持拖拽文件到终端）\n";
            std::cout << "  [名字]  可选，导入后重命名为这个名字（自动补扩展名）\n";
            return 0;
        }
        std::string inputPath = argv[2];
        std::string newName = (argc >= 4) ? argv[3] : "";
        CmdAddFont(inputPath, newName);
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

        CmdUpdate(debugMode, extraFlags);
    } else if (cmd == "version" || cmd == "-v" || cmd == "--version") {
        std::cout << "aurinko " << AURINKO_VERSION << "\n";
    } else {
        PrintUsage();
    }

    return 0;
}