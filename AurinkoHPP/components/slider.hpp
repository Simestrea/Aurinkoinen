#pragma once
#include "raylib.h"
#include <aurinko/init.hpp>
#include <aurinko/components/text.hpp>
#include <string>
#include <algorithm>
#include <cmath>

namespace aur {

// ==========================================
// Slider：拖动式数值调节器
// ==========================================
class Slider {
private:
    // ---------- 布局 ----------
    float x_{0.0f};
    float y_{0.0f};
    float w_{200.0f};
    float h_{24.0f};
    float labelWidth_{110.0f};

    // ---------- 内容 ----------
    std::string label_{};
    double value_{0.0};
    double min_{0.0};
    double max_{1.0};
    double step_{0.0};    // 0 = 连续

    // ---------- 字体 ----------
    std::string fontName_{};
    int labelSize_{16};
    int valueSize_{16};

    // ---------- 样式 ----------
    Color trackColor_{220, 220, 220, 255};
    Color fillColor_{33, 150, 243, 255};
    Color thumbColor_{255, 255, 255, 255};
    Color thumbBorderColor_{33, 150, 243, 255};
    Color labelColor_{60, 60, 60, 255};
    Color valueColor_{2, 60, 120, 255};

    // ---------- 状态 ----------
    bool visible_{true};
    bool enabled_{true};
    bool dragging_{false};
    bool changedThisFrame_{false};

    // 值 → 归一化位置 0..1
    float ValueToFrac() const {
        if (max_ <= min_) return 0.0f;
        return (float)((value_ - min_) / (max_ - min_));
    }

    void ApplyStep() {
        if (step_ > 0.0) {
            value_ = min_ + std::round((value_ - min_) / step_) * step_;
            value_ = std::clamp(value_, min_, max_);
        }
    }

    // 手柄半径
    float ThumbRadius() const { return h_ * 0.4f; }

    // 轨道区域（去掉左侧标签和右侧数值的宽度）
    Rectangle TrackBounds() const {
        const float pad = 4.0f;
        return {
            x_ + labelWidth_ + pad,
            y_ + h_ * 0.5f - 3.0f,
            w_ - labelWidth_ - 90.0f - pad * 2.0f,   // 右边留 90 给数值
            6.0f
        };
    }

    bool HitThumb(Vector2 p) const {
        Rectangle t = TrackBounds();
        float cx = t.x + t.width * ValueToFrac();
        float cy = t.y + t.height * 0.5f;
        float r  = ThumbRadius() + 2.0f;
        float dx = p.x - cx;
        float dy = p.y - cy;
        return dx * dx + dy * dy <= r * r;
    }

    bool HitTrack(Vector2 p) const {
        Rectangle t = TrackBounds();
        // 增大点击区域，方便点击
        return p.x >= t.x - 4.0f && p.x <= t.x + t.width + 4.0f &&
               p.y >= t.y - 8.0f && p.y <= t.y + t.height + 8.0f;
    }

    // 把鼠标 x 位置映射成值
    void SetFromMouseX(float mx) {
        Rectangle t = TrackBounds();
        float frac = (mx - t.x) / t.width;
        frac = std::clamp(frac, 0.0f, 1.0f);
        value_ = min_ + frac * (max_ - min_);
        ApplyStep();
        changedThisFrame_ = true;
    }

public:
    Slider() = default;

    Slider(const std::string& label,
           float x, float y, float w, float h,
           double minVal, double maxVal, double init)
        : x_{x}, y_{y}, w_{w}, h_{h},
          label_{label},
          value_{std::clamp(init, minVal, maxVal)},
          min_{minVal}, max_{maxVal} {}

    // ==========================================
    // 设置
    // ==========================================
    void SetLabel(const std::string& s) { label_ = s; }
    void SetRange(double minVal, double maxVal) {
        min_ = minVal;
        max_ = maxVal;
        value_ = std::clamp(value_, min_, max_);
    }
    void SetValue(double v) {
        value_ = std::clamp(v, min_, max_);
        ApplyStep();
    }
    void SetStep(double s) { step_ = s; ApplyStep(); }
    void SetPosition(float x, float y) { x_ = x; y_ = y; }
    void SetSize(float w, float h) { w_ = w; h_ = h; }
    void SetLabelWidth(float w) { labelWidth_ = w; }
    void SetFont(const std::string& f) { fontName_ = f; }
    void SetFontSize(int labelSz, int valueSz) {
        labelSize_ = labelSz;
        valueSize_ = valueSz;
    }

    // ==========================================
    // 样式
    // ==========================================
    void SetTrackColor(Color c)       { trackColor_ = c; }
    void SetFillColor(Color c)        { fillColor_ = c; }
    void SetThumbColor(Color c)       { thumbColor_ = c; }
    void SetThumbBorderColor(Color c) { thumbBorderColor_ = c; }
    void SetLabelColor(Color c)       { labelColor_ = c; }
    void SetValueColor(Color c)       { valueColor_ = c; }

    // ==========================================
    // 状态
    // ==========================================
    void Show() { visible_ = true; }
    void Hide() { visible_ = false; }
    void SetEnabled(bool e) {
        enabled_ = e;
        if (!e) dragging_ = false;
    }
    bool IsEnabled() const noexcept { return enabled_; }
    bool IsVisible() const noexcept { return visible_; }
    bool IsDragging() const noexcept { return dragging_; }

    double Value() const noexcept { return value_; }

    // ==========================================
    // 交互
    // ==========================================
    // 本帧是否拖动过（消费型：返回一次后清除标记）
    bool Changed() {
        bool c = changedThisFrame_;
        changedThisFrame_ = false;
        return c;
    }

    // ==========================================
    // 绘制
    // ==========================================
    void Draw() {
        if (!visible_) return;

        const Vector2 mouse = detail::virtualScreen.GetVirtualMousePosition();

        // ---------- 输入处理 ----------
        if (enabled_) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                if (HitThumb(mouse)) {
                    dragging_ = true;
                } else if (HitTrack(mouse)) {
                    dragging_ = true;
                    SetFromMouseX(mouse.x);
                }
            }
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                dragging_ = false;
            }
            if (dragging_ && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                SetFromMouseX(mouse.x);
            }
        } else {
            dragging_ = false;
        }

        // ---------- 绘制 ----------
        Font font = fontName_.empty() ? GetFontDefault()
                                      : detail::fontPool.GetOrLoad(fontName_);

        // 标签
        if (!label_.empty()) {
            float ty = y_ + (h_ - labelSize_) * 0.5f;
            DrawTextEx(font, label_.c_str(),
                       { x_, ty },
                       (float)labelSize_, 1.0f, labelColor_);
        }

        // 轨道
        Rectangle track = TrackBounds();
        DrawRectangleRec(track, trackColor_);

        // 已填充部分
        float frac = ValueToFrac();
        Rectangle filled = track;
        filled.width = track.width * frac;
        DrawRectangleRec(filled, fillColor_);

        // 手柄
        float thumbX = track.x + track.width * frac;
        float thumbY = track.y + track.height * 0.5f;
        float r = ThumbRadius();
        DrawCircle((int)thumbX, (int)thumbY, r, thumbColor_);
        DrawCircleLines((int)thumbX, (int)thumbY, r, thumbBorderColor_);

        // 数值显示
        char buf[48];
        if (max_ - min_ >= 1000.0) {
            snprintf(buf, sizeof(buf), "%.0f", value_);
        } else if (max_ - min_ >= 1.0) {
            snprintf(buf, sizeof(buf), "%.2f", value_);
        } else {
            snprintf(buf, sizeof(buf), "%.4g", value_);
        }
        float vx = x_ + w_ - 80.0f;
        float vy = y_ + (h_ - valueSize_) * 0.5f;
        DrawTextEx(font, buf, { vx, vy },
                   (float)valueSize_, 1.0f, valueColor_);
    }
};

} // namespace aur