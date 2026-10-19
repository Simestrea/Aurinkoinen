#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
patch_aurinko_101.py —— aurinko v0.6.0 → v1.0.1
  [1] version.hpp: 0.6.0 → 1.0.1
  [2] commands.hpp: 加 CmdDeleteCharacter / CmdListCharacters
  [3] aurinko.cpp: 加 delete / list 命令 dispatch
  [4] PrintUsage 更新帮助
"""

import sys
from pathlib import Path

# ============================================================
# 版本号
# ============================================================
VER_OLD = 'inline constexpr const char* AURINKO_VERSION = "0.6.0";'
VER_NEW = 'inline constexpr const char* AURINKO_VERSION = "1.0.1";'

# ============================================================
# 新命令
# ============================================================
CMD_NEW_CODE = r'''
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

'''

CMD_ANCHOR = '''// ==========================================
// Command: create stylesheet (empty)
//   返回: 0 = 成功, 1 = 失败
// =========================================='''

# ============================================================
# aurinko.cpp dispatch
# ============================================================
CPP_ANCHOR = '''    } else if (cmd == "addfont") {'''

CPP_NEW = '''    } else if (cmd == "delete" || cmd == "remove" || cmd == "rm") {
        if (argc < 3) {
            std::cout << "Usage: aurinko delete <name>\\n";
            return 1;
        }
        return CmdDeleteCharacter(argv[2]);
    } else if (cmd == "list" || cmd == "ls") {
        return CmdListCharacters();
    } else if (cmd == "addfont") {'''

# ============================================================
# PrintUsage 加两行
# ============================================================
USAGE_OLD = '''    std::cout << "  aurinko create-style <name>     Create an empty stylesheet\\n";'''
USAGE_NEW = '''    std::cout << "  aurinko create-style <name>     Create an empty stylesheet\\n";
    std::cout << "  aurinko delete <name>           Delete a character (with confirmation)\\n";
    std::cout << "  aurinko list                    List all characters\\n";'''


def patch_file(path: Path, old: str, new: str, label: str):
    if not path.is_file():
        print(f"[FAIL] {label}  文件不存在: {path}")
        return False
    try:
        raw = path.read_text(encoding="utf-8")
    except UnicodeDecodeError:
        raw = path.read_text(encoding="gbk", errors="replace")

    if new in raw and new != old:
        print(f"[SKIP] {label}")
        return True
    if old not in raw:
        print(f"[FAIL] {label}  锚点未找到")
        return False

    raw = raw.replace(old, new, 1)

    bak = Path(str(path) + ".bak101")
    if not bak.is_file():
        bak.write_text(raw, encoding="utf-8")

    path.write_text(raw, encoding="utf-8")
    print(f"[ OK ] {label}")
    return True


def main():
    root = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(".")
    print(f"aurinko 源码根: {root.resolve()}")

    results = []

    # [1] 版本号
    results.append(patch_file(
        root / "version.hpp", VER_OLD, VER_NEW, "版本号 → 1.0.1"))

    # [2] 加新命令
    results.append(patch_file(
        root / "commands.hpp", CMD_ANCHOR, CMD_NEW_CODE + CMD_ANCHOR,
        "commands.hpp 加 delete / list 命令"))

    # [3] aurinko.cpp dispatch
    results.append(patch_file(
        root / "aurinko.cpp", CPP_ANCHOR, CPP_NEW,
        "aurinko.cpp 加 dispatch"))

    # [4] PrintUsage
    results.append(patch_file(
        root / "aurinko.cpp", USAGE_OLD, USAGE_NEW,
        "PrintUsage 加提示"))

    print("-" * 60)
    if all(results):
        print("[完成] 全部成功")
        print()
        print("下一步：重新编译 aurinko：")
        print("  g++ aurinko.cpp -o aurinko.exe -std=c++20 -Wall -Wextra")
        print("  copy aurinko.exe ..\\my-aurinko-ide\\w64devkit\\w64devkit\\bin\\")
        print("  copy aurinko.exe ..\\my-aurinko-ide\\build\\w64devkit\\w64devkit\\bin\\")
        return 0
    print("[!] 有失败项，检查上面的输出")
    return 2


if __name__ == "__main__":
    sys.exit(main())