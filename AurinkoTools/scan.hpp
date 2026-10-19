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
    std::vector<std::string> paramTypes;   // normalized parameter types (original order)
    bool isClone;
    bool isAction;
};

struct CharFunctions {
    std::string name;
    std::vector<DetectedFunction> functions;
};

// ==========================================
// Helpers
// ==========================================

inline bool EndsWith(const std::string& s, const std::string& suffix) {
    if (s.size() < suffix.size()) return false;
    return s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

// Normalize "const aur::Context & ctx" to "aur::Context".
// Steps: strip const/volatile -> strip & and * -> collapse spaces -> drop trailing var name.
inline std::string NormalizeType(const std::string& raw) {
    std::string s = Trim(raw);

    // 1) remove const / volatile (word boundary)
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

    // 2) remove & and *
    std::string noRefs;
    noRefs.reserve(s.size());
    for (char c : s) {
        if (c == '&' || c == '*') continue;
        noRefs += c;
    }

    // 3) collapse multiple spaces
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

    // 4) drop trailing variable name if it's an identifier
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

// Split a parameter list on commas (tracks < > ( ) depth to avoid splitting templates)
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
// Function scan
// ==========================================
// Supports "inline Task" / "inline Clone" / "inline aur::Task" / "inline aur::Clone" / "inline aur::Action"
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

        bool isClone  = false;
        bool isAction = false;
        if (line.compare(p, 5, "Task ") == 0) {
            isClone = false; p += 5;
        } else if (line.compare(p, 6, "Clone ") == 0) {
            isClone = true;  p += 6;
        } else if (line.compare(p, 7, "Action ") == 0) {
            isAction = true; p += 7;
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

        funcs.push_back({ name, std::move(paramTypes), isClone, isAction });
    }
    return funcs;
}

// ==========================================
// Forbidden symbol scan
// ==========================================
struct ScanViolation {
    std::string file;
    int line;
    std::string text;
};

// Scan one hpp for lines referencing forbidden symbols (comments ignored)
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

        auto firstNonSpace = line.find_first_not_of(" \t");
        if (firstNonSpace == std::string::npos) continue;

        // Skip full-line comments
        if (line[firstNonSpace] == '/' &&
            firstNonSpace + 1 < line.size() &&
            line[firstNonSpace + 1] == '/') {
            continue;
        }

        // Strip trailing comments before checking
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