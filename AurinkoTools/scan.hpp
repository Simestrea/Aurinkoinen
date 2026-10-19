#pragma once
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <cctype>
#include "utils.hpp"

namespace fs = std::filesystem;

struct DetectedFunction {
    std::string name;
    std::vector<std::string> paramTypes;   // 规范化后的参数类型（按原顺序）
    bool isClone;
};

struct CharFunctions {
    std::string name;
    std::vector<DetectedFunction> functions;
};

// ==========================================
// 小工具
// ==========================================

inline bool EndsWith(const std::string& s, const std::string& suffix) {
    if (s.size() < suffix.size()) return false;
    return s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

// 把 "const aur::Context & ctx" 规范化成 "aur::Context"
// 步骤：去 const/volatile → 去 & 和 * → 折叠空格 → 剥掉尾部变量名
inline std::string NormalizeType(const std::string& raw) {
    std::string s = Trim(raw);

    // 1) 去掉 const / volatile（按词边界匹配）
    auto stripWord = [](std::string& str, const std::string& word) {
        size_t pos;
        while ((pos = str.find(word)) != std::string::npos) {
            size_t after = pos + word.size();
            bool leftOk = (pos == 0) ||
                          !std::isalnum(static_cast<unsigned char>(str[pos - 1]));
            bool rightOk = (after >= str.size()) ||
                           !std::isalnum(static_cast<unsigned char>(str[after]));
            if (leftOk && rightOk) str.erase(pos, word.size());
            else break;
        }
    };
    stripWord(s, "const");
    stripWord(s, "volatile");

    // 2) 去掉 & 和 *
    std::string noRefs;
    noRefs.reserve(s.size());
    for (char c : s) {
        if (c == '&' || c == '*') continue;
        noRefs += c;
    }

    // 3) 折叠多余空格
    std::string collapsed;
    collapsed.reserve(noRefs.size());
    bool lastWasSpace = false;
    for (char c : noRefs) {
        if (std::isspace(static_cast<unsigned char>(c))) {
            if (!lastWasSpace) collapsed += ' ';
            lastWasSpace = true;
        } else {
            collapsed += c;
            lastWasSpace = false;
        }
    }
    collapsed = Trim(collapsed);

    // 4) 剥掉尾部变量名：最后一个空格后如果全是标识符，那它是变量名
    auto lastSpace = collapsed.find_last_of(' ');
    if (lastSpace != std::string::npos) {
        std::string tail = collapsed.substr(lastSpace + 1);
        bool isIdent = !tail.empty();
        for (char c : tail) {
            if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_') {
                isIdent = false;
                break;
            }
        }
        if (isIdent) collapsed = Trim(collapsed.substr(0, lastSpace));
    }

    return collapsed;
}

// 按逗号分割参数列表（跟踪 < > ( ) 深度，避免拆模板参数）
inline std::vector<std::string> SplitParams(const std::string& args) {
    std::vector<std::string> out;
    std::string cur;
    int depth = 0;
    for (char c : args) {
        if (c == '<' || c == '(') ++depth;
        else if (c == '>' || c == ')') --depth;

        if (c == ',' && depth == 0) {
            out.push_back(cur);
            cur.clear();
        } else {
            cur += c;
        }
    }
    if (!Trim(cur).empty()) out.push_back(cur);
    return out;
}

// ==========================================
// 函数扫描
// ==========================================
// 支持 "inline Task" / "inline Clone" / "inline aur::Task" / "inline aur::Clone"
inline std::vector<DetectedFunction> DetectFunctions(const fs::path& hppPath) {
    std::vector<DetectedFunction> funcs;
    std::ifstream in(hppPath);
    if (!in.is_open()) return funcs;

    std::string line;
    while (std::getline(in, line)) {
        auto inlinePos = line.find("inline ");
        if (inlinePos == std::string::npos) continue;

        size_t p = inlinePos + 7;
        while (p < line.size() && std::isspace(static_cast<unsigned char>(line[p]))) p++;

        if (line.compare(p, 5, "aur::") == 0) p += 5;

        bool isClone = false;
        if (line.compare(p, 5, "Task ") == 0) {
            isClone = false; p += 5;
        } else if (line.compare(p, 6, "Clone ") == 0) {
            isClone = true;  p += 6;
        } else {
            continue;
        }

        auto nameStart = p;
        auto nameEnd = line.find('(', nameStart);
        if (nameEnd == std::string::npos) continue;

        auto argsEnd = line.find(')', nameEnd);
        if (argsEnd == std::string::npos) continue;

        std::string name = Trim(line.substr(nameStart, nameEnd - nameStart));
        if (name.empty()) continue;

        std::string args = line.substr(nameEnd + 1, argsEnd - nameEnd - 1);

        std::vector<std::string> paramTypes;
        for (auto& raw : SplitParams(args)) {
            std::string norm = NormalizeType(raw);
            if (!norm.empty() && norm != "void") {
                paramTypes.push_back(norm);
            }
        }

        funcs.push_back({ name, std::move(paramTypes), isClone });
    }
    return funcs;
}

// ==========================================
// 违规扫描
// ==========================================
struct ScanViolation {
    std::string file;
    int line;
    std::string text;
};

// 扫描单个 hpp，找出引用了禁止符号的行（注释里的不算）
inline std::vector<ScanViolation> CheckForbidden(
    const fs::path& hppPath,
    const std::vector<std::string>& forbidden)
{
    std::vector<ScanViolation> violations;
    std::ifstream in(hppPath);
    if (!in.is_open()) return violations;

    std::string line;
    int lineNo = 0;
    while (std::getline(in, line)) {
        lineNo++;

        // 跳过空行
        auto firstNonSpace = line.find_first_not_of(" \t");
        if (firstNonSpace == std::string::npos) continue;

        // 跳过整行注释
        if (line[firstNonSpace] == '/' &&
            firstNonSpace + 1 < line.size() &&
            line[firstNonSpace + 1] == '/') {
            continue;
        }

        // 去掉行尾注释后再判断
        std::string code = line;
        auto commentPos = code.find("//");
        if (commentPos != std::string::npos) {
            code = code.substr(0, commentPos);
        }

        for (auto& bad : forbidden) {
            if (code.find(bad) != std::string::npos) {
                violations.push_back({ hppPath.string(), lineNo, Trim(line) });
                break;
            }
        }
    }
    return violations;
}