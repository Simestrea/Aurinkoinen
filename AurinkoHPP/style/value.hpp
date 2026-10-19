#pragma once
#include "raylib.h"
#include <string>
#include <optional>
#include <unordered_map>
#include <cctype>

namespace aur::style {

// ---------- ÃüÃûÑÕÉ« ----------
inline std::optional<Color> NamedColor(const std::string& name) {
    static const std::unordered_map<std::string, Color> table = {
        {"white", WHITE}, {"black", BLACK},
        {"red", RED}, {"green", GREEN}, {"blue", BLUE},
        {"yellow", YELLOW}, {"orange", ORANGE},
        {"gray", GRAY}, {"grey", GRAY},
        {"lightgray", LIGHTGRAY}, {"lightgrey", LIGHTGRAY},
        {"darkgray", DARKGRAY}, {"darkgrey", DARKGRAY},
        {"skyblue", SKYBLUE}, {"darkblue", DARKBLUE},
        {"transparent", Color{0, 0, 0, 0}},
    };
    auto it = table.find(name);
    if (it == table.end()) return std::nullopt;
    return it->second;
}

// ---------- #RRGGBB / #RRGGBBAA ----------
inline std::optional<Color> ParseHexColor(const std::string& s) {
    if (s.empty() || s[0] != '#') return std::nullopt;
    std::string h = s.substr(1);
    if (h.size() != 6 && h.size() != 8) return std::nullopt;

    auto hv = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    };
    auto pair = [&](char a, char b) -> int {
        int hi = hv(a), lo = hv(b);
        if (hi < 0 || lo < 0) return -1;
        return hi * 16 + lo;
    };
    int r = pair(h[0], h[1]);
    int g = pair(h[2], h[3]);
    int b = pair(h[4], h[5]);
    int a = (h.size() == 8) ? pair(h[6], h[7]) : 255;
    if (r < 0 || g < 0 || b < 0 || a < 0) return std::nullopt;
    return Color{
        (unsigned char)r, (unsigned char)g,
        (unsigned char)b, (unsigned char)a
    };
}

// ---------- Öµ ----------
struct Value {
    std::string raw;
    Value() = default;
    Value(const std::string& s) : raw(s) {}

    std::optional<Color> AsColor() const {
        std::string s = Trim(raw);
        if (s.empty()) return std::nullopt;
        if (s[0] == '#') return ParseHexColor(s);
        std::string lower;
        lower.reserve(s.size());
        for (char c : s) lower += (char)std::tolower((unsigned char)c);
        return NamedColor(lower);
    }
    std::optional<int> AsInt() const {
        try { return std::stoi(raw); } catch (...) { return std::nullopt; }
    }
    std::optional<float> AsFloat() const {
        try { return std::stof(raw); } catch (...) { return std::nullopt; }
    }
    std::optional<bool> AsBool() const {
        std::string lower;
        lower.reserve(raw.size());
        for (char c : raw) lower += (char)std::tolower((unsigned char)c);
        if (lower == "true"  || lower == "1" || lower == "yes" || lower == "on")  return true;
        if (lower == "false" || lower == "0" || lower == "no"  || lower == "off") return false;
        return std::nullopt;
    }
    std::string AsString() const { return raw; }

private:
    static std::string Trim(const std::string& s) {
        size_t a = 0, b = s.size();
        while (a < b && std::isspace((unsigned char)s[a])) ++a;
        while (b > a && std::isspace((unsigned char)s[b - 1])) --b;
        return s.substr(a, b - a);
    }
};

} // namespace aur::style
