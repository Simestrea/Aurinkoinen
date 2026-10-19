#pragma once
#include "raylib.h"
#include "init.hpp"
#include "text.hpp"
#include <string>

namespace aur {

// ==========================================
// Panel：带边框和标题的矩形面板
//
// 特点：
//   - 背景 + 边框 + 可选标题
//   - 可选圆角
//   - 提供 ContentTopLeft / ContentSize，方便放置内部元素
//
// 用法：
//   aur::Panel panel("仿真参数", 790, 90, 480, 530);
//   panel.SetCornerRadius(8.0f);
//   panel.Draw();
//
//   Vector2 p = panel.ContentTopLeft();
//   // 在 p.x, p.y 处放第一个控件
// ==========================================
class Panel {
private:
    float x_{0.0f};
    float y_{0.0f};
    float w_{100.0f};
    float h_{100.0f};

    std::string title_{};
    std::string fontName_{};
    int   titleSize_{18};
    float titlePadding_{12.0f};

    Color bgColor_{255, 255, 255, 255};
    Color borderColor_{200, 200, 200, 255};
    Color titleColor_{2, 60, 120, 255};
    float borderWidth_{1.0f};

    float cornerRadius_{0.0f};
    int   cornerSegments_{8};

    bool visible_{true};

public:
    Panel() = default;

    Panel(float x, float y, float w, float h)
        : x_{x}, y_{y}, w_{w}, h_{h} {}

    Panel(const std::string& title, float x, float y, float w, float h)
        : x_{x}, y_{y}, w_{w}, h_{h}, title_{title} {}

    // ==========================================
    // 布局
    // ==========================================
    void SetPosition(float x, float y) { x_ = x; y_ = y; }
    void SetSize(float w, float h) { w_ = w; h_ = h; }
    void SetTitle(const std::string& s) { title_ = s; }
    void SetTitleSize(int s) { titleSize_ = s; }
    void SetTitlePadding(float p) { titlePadding_ = p; }
    void SetFont(const std::string& f) { fontName_ = f; }

    Rectangle Bounds() const noexcept { return { x_, y_, w_, h_ }; }

    // 内容区（跳过标题行）
    Vector2 ContentTopLeft() const {
        float top = y_ + titlePadding_;
        if (!title_.empty()) top += titleSize_ + 8.0f;
        return { x_ + titlePadding_, top };
    }
    Vector2 ContentSize() const {
        float top = titlePadding_ * 2.0f;
        if (!title_.empty()) top += titleSize_ + 8.0f;
        return { w_ - titlePadding_ * 2.0f, h_ - top };
    }

    // ==========================================
    // 样式
    // ==========================================
    void SetBgColor(Color c)     { bgColor_ = c; }
    void SetBorderColor(Color c) { borderColor_ = c; }
    void SetTitleColor(Color c)  { titleColor_ = c; }
    void SetBorderWidth(float w) { borderWidth_ = w; }

    void SetCornerRadius(float r, int segments = 8) {
        cornerRadius_ = r;
        cornerSegments_ = segments;
    }

    // ==========================================
    // 可见性
    // ==========================================
    void Show() { visible_ = true; }
    void Hide() { visible_ = false; }
    void ToggleVisible() { visible_ = !visible_; }
    bool IsVisible() const noexcept { return visible_; }

    // ==========================================
    // 绘制
    // ==========================================
    void Draw() const {
        if (!visible_) return;

        const Rectangle r{ x_, y_, w_, h_ };

        if (cornerRadius_ > 0.0f) {
            DrawRectangleRounded(r, cornerRadius_ / std::min(w_, h_),
                                 cornerSegments_, bgColor_);
            if (borderWidth_ > 0.0f) {
                DrawRectangleRoundedLinesEx(r, cornerRadius_ / std::min(w_, h_),
                            cornerSegments_,
                            borderWidth_, borderColor_);
            }
        } else {
            DrawRectangleRec(r, bgColor_);
            if (borderWidth_ > 0.0f) {
                DrawRectangleLinesEx(r, borderWidth_, borderColor_);
            }
        }

        if (title_.empty()) return;

        Font font = fontName_.empty() ? GetFontDefault()
                                      : detail::fontPool.GetOrLoad(fontName_);
        DrawTextEx(font, title_.c_str(),
                   { x_ + titlePadding_, y_ + titlePadding_ },
                   (float)titleSize_, 1.0f, titleColor_);
    }
};

} // namespace aur