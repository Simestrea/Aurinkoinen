#pragma once
#include "value.hpp"
#include <string>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <iostream>

namespace aur::style {

// ---------- 一组属性 ----------
struct Style {
    std::unordered_map<std::string, Value> props;

    Style& Set(const std::string& k, const std::string& v) {
        props[k] = Value{v};
        return *this;
    }
    bool Has(const std::string& k) const {
        return props.find(k) != props.end();
    }

    std::optional<Color> GetColor(const std::string& k) const {
        auto it = props.find(k);
        if (it == props.end()) return std::nullopt;
        return it->second.AsColor();
    }
    std::optional<int> GetInt(const std::string& k) const {
        auto it = props.find(k);
        if (it == props.end()) return std::nullopt;
        return it->second.AsInt();
    }
    std::optional<float> GetFloat(const std::string& k) const {
        auto it = props.find(k);
        if (it == props.end()) return std::nullopt;
        return it->second.AsFloat();
    }
    std::optional<bool> GetBool(const std::string& k) const {
        auto it = props.find(k);
        if (it == props.end()) return std::nullopt;
        return it->second.AsBool();
    }
    std::optional<std::string> GetString(const std::string& k) const {
        auto it = props.find(k);
        if (it == props.end()) return std::nullopt;
        return it->second.AsString();
    }
};

// ---------- 样式表 ----------
class StyleSheet {
public:
    bool Load(const std::string& path) {
        std::ifstream in(path, std::ios::binary);
        if (!in.is_open()) {
            std::cerr << "[style] cannot open: " << path << "\n";
            return false;
        }
        std::stringstream ss;
        ss << in.rdbuf();
        return Parse(ss.str());
    }

    bool Parse(const std::string& text) {
        std::istringstream in(text);
        std::string line;
        std::string current;
        int lineNo = 0;

        while (std::getline(in, line)) {
            ++lineNo;
            if (!line.empty() && line.back() == '\r') line.pop_back();

            std::string t = Trim(line);
            if (t.empty()) continue;
            if (t[0] == '#' || t[0] == ';') continue;
            if (t.size() >= 2 && t[0] == '/' && t[1] == '/') continue;

            if (t[0] == '[') {
                auto close = t.find(']');
                if (close == std::string::npos) {
                    std::cerr << "[style] line " << lineNo << ": missing ]\n";
                    continue;
                }
                current = Trim(t.substr(1, close - 1));
                if (!rules_.count(current)) rules_[current] = Style{};
                continue;
            }

            auto eq = t.find('=');
            if (eq == std::string::npos) {
                std::cerr << "[style] line " << lineNo << ": missing =\n";
                continue;
            }
            if (current.empty()) {
                std::cerr << "[style] line " << lineNo << ": no [section]\n";
                continue;
            }
            std::string key   = Trim(t.substr(0, eq));
            std::string value = Trim(t.substr(eq + 1));

            if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
                value = value.substr(1, value.size() - 2);
            }
            rules_[current].props[key] = Value{value};
        }
        return true;
    }

    bool Has(const std::string& sel) const {
        return rules_.count(sel) > 0;
    }
    const Style* Find(const std::string& sel) const {
        auto it = rules_.find(sel);
        if (it == rules_.end()) return nullptr;
        return &it->second;
    }
    Style GetOrEmpty(const std::string& sel) const {
        auto p = Find(sel);
        return p ? *p : Style{};
    }

    const std::unordered_map<std::string, Style>& All() const { return rules_; }
    void Clear() { rules_.clear(); }
    void Dump() const {
        for (auto& [k, v] : rules_) {
            std::cout << "[" << k << "]\n";
            for (auto& [pk, pv] : v.props) {
                std::cout << "  " << pk << " = " << pv.raw << "\n";
            }
        }
    }

    // ---------- 给组件应用样式（模板定义在 apply.hpp） ----------
    template<typename T>
    void Apply(T& component, const std::string& selector) const;

private:
    std::unordered_map<std::string, Style> rules_;

    static std::string Trim(const std::string& s) {
        size_t a = 0, b = s.size();
        while (a < b && std::isspace((unsigned char)s[a])) ++a;
        while (b > a && std::isspace((unsigned char)s[b - 1])) --b;
        return s.substr(a, b - a);
    }
};

} // namespace aur::style
