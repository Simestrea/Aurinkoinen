#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <cctype>

inline std::string Ask(const std::string& prompt) {
    std::cout << prompt;
    std::string line;
    std::getline(std::cin, line);
    return line;
}

inline std::string Trim(const std::string& s) {
    std::string result = s;
    while (!result.empty() && std::isspace(static_cast<unsigned char>(result.front())))
        result.erase(result.begin());
    while (!result.empty() && std::isspace(static_cast<unsigned char>(result.back())))
        result.pop_back();
    return result;
}

inline std::string StripQuotes(const std::string& s) {
    std::string result = Trim(s);
    if (result.size() >= 2 && result.front() == '"' && result.back() == '"') {
        result = result.substr(1, result.size() - 2);
    }
    return result;
}

inline std::string ToLower(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return result;
}

inline std::string FloatLit(float v) {
    std::ostringstream oss;
    oss << v;
    std::string s = oss.str();
    if (s.find('.') == std::string::npos &&
        s.find('e') == std::string::npos &&
        s.find('E') == std::string::npos) {
        s += ".0";
    }
    return s;
}