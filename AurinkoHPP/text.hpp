#pragma once
#include "raylib.h"
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>

namespace aur {

// ==========================================
// FontPool：共享字体资源
// 同一个字体文件只加载一次
// ==========================================
class FontPool {
private:
    std::unordered_map<std::string, Font> fonts_;

    // 常用码点集合（含中文）
    static std::vector<int>& CommonCodepoints() {
        static std::vector<int> cp = []{
            std::vector<int> v;
            v.reserve(9500);
            for (int c = 32;     c <  127;  ++c) v.push_back(c);   // ASCII
            for (int c = 0x0370; c <= 0x03FF; ++c) v.push_back(c); // 希腊字母
            for (int c = 0x2000; c <= 0x206F; ++c) v.push_back(c); // 通用标点
            for (int c = 0x2070; c <= 0x209F; ++c) v.push_back(c); // 上下标
            for (int c = 0x2190; c <= 0x21FF; ++c) v.push_back(c); // 箭头
            for (int c = 0x2200; c <= 0x22FF; ++c) v.push_back(c); // 数学运算符
            for (int c = 0x3000; c <= 0x303F; ++c) v.push_back(c); // CJK 标点
            for (int c = 0xFF00; c <= 0xFFEF; ++c) v.push_back(c); // 全角字符
            for (int c = 0x4E00; c <= 0x9FA5; ++c) v.push_back(c); // CJK 常用汉字
            return v;
        }();
        return cp;
    }

public:
    FontPool() = default;
    ~FontPool() { UnloadAll(); }

    FontPool(const FontPool&) = delete;
    FontPool& operator=(const FontPool&) = delete;

    Font Load(const std::string& key, const std::string& path, int baseSize = 64) {
        auto it = fonts_.find(key);
        if (it != fonts_.end()) return it->second;

        std::vector<int>& cps = CommonCodepoints();
        Font f = LoadFontEx(path.c_str(), baseSize, cps.data(), (int)cps.size());
        if (f.texture.id == 0) {
            TraceLog(LOG_WARNING, "Font load failed: %s", path.c_str());
            return GetFontDefault();
        }
        SetTextureFilter(f.texture, TEXTURE_FILTER_BILINEAR);   // ★
        fonts_[key] = f;
        return f;
    }

    Font GetOrLoad(const std::string& key) {
        auto it = fonts_.find(key);
        if (it != fonts_.end()) return it->second;

        std::vector<int>& cps = CommonCodepoints();
        Font f = LoadFontEx(key.c_str(), 64, cps.data(), (int)cps.size());
        if (f.texture.id == 0) {
            TraceLog(LOG_WARNING, "Font load failed: %s", key.c_str());
            return GetFontDefault();
        }
        SetTextureFilter(f.texture, TEXTURE_FILTER_BILINEAR);   // ★
        fonts_[key] = f;
        return f;
    }

    bool Has(const std::string& key) const {
        return fonts_.find(key) != fonts_.end();
    }

    void UnloadAll() {
        Font def = GetFontDefault();
        for (auto& [k, f] : fonts_) {
            if (f.texture.id != def.texture.id) {
                UnloadFont(f);
            }
        }
        fonts_.clear();
    }
};

// ==========================================
// 内部单例
// ==========================================
namespace detail {
    inline FontPool fontPool;
}

// ==========================================
// Text：单条文字
// ==========================================
class Text {
private:
    std::string content_{};
    float x_{0.0f};
    float y_{0.0f};
    int fontSize_{20};
    Color color_{DARKGRAY};
    std::string fontName_{};
    float spacing_{1.0f};
    bool isHidden_{false};
    bool alwaysOnTop_{false};

public:
    Text(const std::string& content,
         float x, float y,
         int fontSize = 20,
         Color color = DARKGRAY,
         const std::string& fontName = "");

    ~Text();

    Text(const Text&) = delete;
    Text& operator=(const Text&) = delete;
    Text(Text&&) = delete;
    Text& operator=(Text&&) = delete;

    void SetText(const std::string& s) { content_ = s; }
    const std::string& GetText() const noexcept { return content_; }

    void SetPosition(float x, float y) { x_ = x; y_ = y; }
    void SetX(float x) { x_ = x; }
    void SetY(float y) { y_ = y; }
    float GetX() const noexcept { return x_; }
    float GetY() const noexcept { return y_; }

    void SetFontSize(int s) { fontSize_ = s; }
    void SetColor(Color c) { color_ = c; }
    void SetFont(const std::string& fontPath) { fontName_ = fontPath; }
    void SetSpacing(float s) { spacing_ = s; }

    void Show() { isHidden_ = false; }
    void Hide() { isHidden_ = true; }
    void ToggleHidden() { isHidden_ = !isHidden_; }
    bool IsHidden() const noexcept { return isHidden_; }

    void SetAlwaysOnTop(bool on) { alwaysOnTop_ = on; }
    bool IsAlwaysOnTop() const noexcept { return alwaysOnTop_; }

    float MeasureWidth() const {
        if (content_.empty()) return 0.0f;
        Font f = fontName_.empty() ? GetFontDefault()
                                   : detail::fontPool.GetOrLoad(fontName_);
        return MeasureTextEx(f, content_.c_str(),
                             (float)fontSize_, spacing_).x;
    }

    void Draw() const {
        if (isHidden_) return;
        if (content_.empty()) return;

        Font f = fontName_.empty() ? GetFontDefault()
                                   : detail::fontPool.GetOrLoad(fontName_);
        DrawTextEx(f, content_.c_str(), { x_, y_ },
                   (float)fontSize_, spacing_, color_);
    }
};

// ==========================================
// 全局注册表
// ==========================================
namespace detail {
    inline std::vector<Text*>& TextRegistry() {
        static std::vector<Text*> reg;
        return reg;
    }
}

inline void RegisterText(Text* t) {
    detail::TextRegistry().push_back(t);
}

inline void UnregisterText(Text* t) {
    auto& reg = detail::TextRegistry();
    reg.erase(std::remove(reg.begin(), reg.end(), t), reg.end());
}

inline void DrawAllTexts() {
    auto& reg = detail::TextRegistry();
    for (auto* t : reg) if (t && !t->IsAlwaysOnTop()) t->Draw();
    for (auto* t : reg) if (t &&  t->IsAlwaysOnTop()) t->Draw();
}

// ==========================================
// Text 构造 / 析构
// ==========================================
inline Text::Text(const std::string& content,
                  float x, float y,
                  int fontSize, Color color,
                  const std::string& fontName)
    : content_{content}, x_{x}, y_{y}, fontSize_{fontSize},
      color_{color}, fontName_{fontName}
{
    RegisterText(this);
}

inline Text::~Text() {
    UnregisterText(this);
}

} // namespace aur