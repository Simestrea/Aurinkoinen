#pragma once
#include "raylib.h"
#include <aurinko/init.hpp>
#include <aurinko/components/text.hpp>
#include <string>
#include <algorithm>
#include <cstdio>

namespace aur {

// ==========================================
// ProgressBar：只读进度条
//
// 特点：
//   - 值域 [0, max]，默认 max = 1
//   - 可选显示百分比文字
//   - 支持圆角
//
// 用法：
//   aur::ProgressBar pb(20, 700, 1240, 8);
//   pb.SetColors(...);
//   pb.SetShowPercent(true);
//
//   while (true) {
//       pb.SetValue(frac);   // 0..1
//       pb.Draw();
//       co_await aur::WaitNextFrame{};
//   }
// ==========================================
class ProgressBar {
private:
    float x_{0.0f};
    float y_{0.0f};
    float w_{100.0f};
    float h_{10.0f};

    float value_{0.0f};      // 当前值
    float max_{1.0f};        // 上界

    // 样式
    Color trackColor_{220, 220, 220, 255};
    Color fillColor_{33, 150, 243, 255};
    Color borderColor_{160, 160, 160, 255};
    Color textColor_{2, 60, 120, 255};
    float borderWidth_{1.0f};

    // 文字
    bool showPercent_{false};
    std::string fontName_{};
    int fontSize_{14};

    // 圆角
    float cornerRadius_{0.0f};
    int cornerSegments_{8};

    bool visible_{true};

    float Fraction() const {
        if (max_ <= 0.0f) return 0.0f;
        return std::clamp(value_ / max_, 0.0f, 1.0f);
    }

public:
    ProgressBar() = default;

    ProgressBar(float x, float y, float w, float h)
        : x_{x}, y_{y}, w_{w}, h_{h} {}

    // ==========================================
    // 值
    // ==========================================
    void SetValue(float v) { value_ = v; }
    float GetValue() const noexcept { return value_; }

    void SetMax(float m) {
        max_ = m > 0.0f ? m : 1.0f;
    }
    float GetMax() const noexcept { return max_; }

    // 0..1
    float GetFraction() const { return Fraction(); }

    void Reset() { value_ = 0.0f; }

    // ==========================================
    // 布局
    // ==========================================
    void SetPosition(float x, float y) { x_ = x; y_ = y; }
    void SetSize(float w, float h) { w_ = w; h_ = h; }

    Rectangle Bounds() const noexcept { return { x_, y_, w_, h_ }; }

    // ==========================================
    // 样式
    // ==========================================
    void SetColors(Color track, Color fill, Color border, Color text) {
        trackColor_ = track;
        fillColor_ = fill;
        borderColor_ = border;
        textColor_ = text;
    }
    void SetTrackColor(Color c)  { trackColor_ = c; }
    void SetFillColor(Color c)   { fillColor_ = c; }
    void SetBorderColor(Color c) { borderColor_ = c; }
    void SetTextColor(Color c)   { textColor_ = c; }
    void SetBorderWidth(float w) { borderWidth_ = w; }

    void SetCornerRadius(float r, int segments = 8) {
        cornerRadius_ = r;
        cornerSegments_ = segments;
    }

    // ==========================================
    // 文字
    // ==========================================
    void SetShowPercent(bool b) { showPercent_ = b; }
    bool IsShowingPercent() const noexcept { return showPercent_; }

    void SetFont(const std::string& f) { fontName_ = f; }
    void SetFontSize(int s) { fontSize_ = s; }

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
        const float frac = Fraction();

        // 轨道
        if (cornerRadius_ > 0.0f) {
            DrawRectangleRounded(r, cornerRadius_ / std::min(w_, h_),
                                 cornerSegments_, trackColor_);
        } else {
            DrawRectangleRec(r, trackColor_);
        }

        // 填充
        if (frac > 0.0f) {
            Rectangle fill = r;
            fill.width = w_ * frac;
            if (cornerRadius_ > 0.0f) {
                // 圆角矩形在部分填充时会出问题，用小一点的圆角
                float round = std::min(cornerRadius_ / std::min(w_, h_), 1.0f);
                DrawRectangleRounded(fill, round, cornerSegments_, fillColor_);
            } else {
                DrawRectangleRec(fill, fillColor_);
            }
        }

        // 边框
        if (borderWidth_ > 0.0f) {
            if (cornerRadius_ > 0.0f) {
                DrawRectangleRoundedLinesEx(r,
                    cornerRadius_ / std::min(w_, h_),
                    cornerSegments_, borderWidth_, borderColor_);
            } else {
                DrawRectangleLinesEx(r, borderWidth_, borderColor_);
            }
        }

        // 百分比文字（居中叠加在进度条上）
        if (showPercent_) {
            char buf[16];
            snprintf(buf, sizeof(buf), "%d%%", (int)(frac * 100.0f + 0.5f));

            Font font = fontName_.empty() ? GetFontDefault()
                                          : detail::fontPool.GetOrLoad(fontName_);
            Vector2 sz = MeasureTextEx(font, buf, (float)fontSize_, 1.0f);
            float tx = x_ + (w_ - sz.x) / 2.0f;
            float ty = y_ + (h_ - sz.y) / 2.0f;
            DrawTextEx(font, buf, { tx, ty },
                       (float)fontSize_, 1.0f, textColor_);
        }
    }
};

} // namespace aur