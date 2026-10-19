#pragma once
#include "raylib.h"
#include <aurinko/init.hpp>
#include <aurinko/components/text.hpp>
#include <aurinko/components/ui_root.hpp>
#include <string>
#include <functional>
#include <algorithm>

namespace aur {

class Button : public Widget {
private:
    float x_{0.0f};
    float y_{0.0f};
    float w_{100.0f};
    float h_{36.0f};
    std::string label_;

    int fontSize_{18};
    std::string fontName_{};

    Color bgNormal_{33, 150, 243, 255};
    Color bgHover_{21, 101, 192, 255};
    Color bgPressed_{13, 71, 161, 255};
    Color bgDisabled_{180, 180, 180, 255};
    Color borderColor_{13, 71, 161, 255};
    Color textColor_{WHITE};
    float borderWidth_{2.0f};

    float cornerRadius_{0.0f};
    int   cornerSegments_{8};

    bool visible_{true};
    bool enabled_{true};

    std::function<void()> onClick_;

    float Roundness() const {
        float shorter = std::min(w_, h_);
        if (shorter <= 0.0f) return 0.0f;
        float r = std::min(cornerRadius_, shorter * 0.5f);
        return r / shorter;
    }

public:
    Button() = default;

    Button(const std::string& label,
           float x, float y, float w, float h,
           int fontSize = 18,
           const std::string& font = "")
        : x_{x}, y_{y}, w_{w}, h_{h},
          label_{label}, fontSize_{fontSize}, fontName_{font} {}

    Rectangle GetBounds() const override {
        return { x_, y_, w_, h_ };
    }

    void SetLabel(const std::string& s) { label_ = s; }
    void SetPosition(float x, float y) { x_ = x; y_ = y; }
    void SetSize(float w, float h) { w_ = w; h_ = h; }
    void SetFontSize(int s) { fontSize_ = s; }
    void SetFont(const std::string& f) { fontName_ = f; }
    void SetBorderWidth(float w) { borderWidth_ = w; }

    void SetCornerRadius(float r, int segments = 8) {
        cornerRadius_ = r;
        cornerSegments_ = segments;
    }

    void SetColors(Color normal, Color hover, Color pressed,
                   Color border, Color text) {
        bgNormal_ = normal;
        bgHover_ = hover;
        bgPressed_ = pressed;
        borderColor_ = border;
        textColor_ = text;
    }
    void SetNormalColor(Color c)   { bgNormal_ = c; }
    void SetHoverColor(Color c)    { bgHover_ = c; }
    void SetPressedColor(Color c)  { bgPressed_ = c; }
    void SetDisabledColor(Color c) { bgDisabled_ = c; }
    void SetBorderColor(Color c)   { borderColor_ = c; }
    void SetTextColor(Color c)     { textColor_ = c; }

    void SetOnClick(std::function<void()> fn) { onClick_ = std::move(fn); }

    void Show() { visible_ = true; }
    void Hide() { visible_ = false; }
    bool IsVisible() const noexcept { return visible_; }

    void SetEnabled(bool e) { enabled_ = e; }
    bool IsEnabled() const noexcept { return enabled_; }

    void OnClick() override {
        if (!visible_ || !enabled_) return;
        if (onClick_) onClick_();
    }

    void Draw() override {
        if (!visible_) return;

        const bool hover = IsHovered() && enabled_;
        const bool pressed = hover && IsMouseButtonDown(MOUSE_BUTTON_LEFT);

        Color bg = bgNormal_;
        if (!enabled_)     bg = bgDisabled_;
        else if (pressed)  bg = bgPressed_;
        else if (hover)    bg = bgHover_;

        const Rectangle r{ x_, y_, w_, h_ };
        const float round = Roundness();

        if (round > 0.0f) {
            DrawRectangleRounded(r, round, cornerSegments_, bg);
            if (borderWidth_ > 0.0f) {
                DrawRectangleRoundedLinesEx(r, round, cornerSegments_,
                                            borderWidth_, borderColor_);
            }
        } else {
            DrawRectangleRec(r, bg);
            if (borderWidth_ > 0.0f) {
                DrawRectangleLinesEx(r, borderWidth_, borderColor_);
            }
        }

        if (label_.empty()) return;

        Font font = fontName_.empty() ? GetFontDefault()
                                      : detail::fontPool.GetOrLoad(fontName_);
        const Vector2 sz = MeasureTextEx(font, label_.c_str(),
                                         (float)fontSize_, 1.0f);
        const float tx = x_ + (w_ - sz.x) / 2.0f;
        const float ty = y_ + (h_ - sz.y) / 2.0f;
        DrawTextEx(font, label_.c_str(), { tx, ty },
                   (float)fontSize_, 1.0f, textColor_);
    }
};

} // namespace aur
