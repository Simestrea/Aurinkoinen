#pragma once
#include "stylesheet.hpp"
#include <sstream>
#include <initializer_list>
#include <aurinko/components/button.hpp>
#include <aurinko/components/panel.hpp>
#include <aurinko/components/progress_bar.hpp>
#include <aurinko/components/textbox.hpp>
#include <aurinko/components/text.hpp>
#include <aurinko/components/slider.hpp>
#include <aurinko/components/table.hpp>
#include <aurinko/components/plot/plot.hpp>

namespace aur::style {

// ==========================================
// 别名辅助：依次尝试多个 key，返回第一个命中的
// ==========================================
inline std::optional<Color> PickColor(const Style& s, std::initializer_list<const char*> keys) {
    for (auto k : keys) if (auto v = s.GetColor(k)) return v;
    return std::nullopt;
}
inline std::optional<int> PickInt(const Style& s, std::initializer_list<const char*> keys) {
    for (auto k : keys) if (auto v = s.GetInt(k)) return v;
    return std::nullopt;
}
inline std::optional<float> PickFloat(const Style& s, std::initializer_list<const char*> keys) {
    for (auto k : keys) if (auto v = s.GetFloat(k)) return v;
    return std::nullopt;
}
inline std::optional<bool> PickBool(const Style& s, std::initializer_list<const char*> keys) {
    for (auto k : keys) if (auto v = s.GetBool(k)) return v;
    return std::nullopt;
}
inline std::optional<std::string> PickString(const Style& s, std::initializer_list<const char*> keys) {
    for (auto k : keys) if (auto v = s.GetString(k)) return v;
    return std::nullopt;
}

// ==========================================
// Button
// ==========================================
inline void ApplyStyle(aur::Button& b, const Style& s) {
    if (auto v = PickColor(s, {"bg", "background", "background-color", "normal-bg"}))
        b.SetNormalColor(*v);
    if (auto v = PickColor(s, {"hover-bg", "hover-background", "hover-color"}))
        b.SetHoverColor(*v);
    if (auto v = PickColor(s, {"pressed-bg", "active-bg", "active-background",
                               "pressed-background"}))
        b.SetPressedColor(*v);
    if (auto v = PickColor(s, {"disabled-bg", "disabled-background"}))
        b.SetDisabledColor(*v);
    if (auto v = PickColor(s, {"border", "border-color"}))
        b.SetBorderColor(*v);
    if (auto v = PickColor(s, {"text", "text-color", "color", "foreground"}))
        b.SetTextColor(*v);
    if (auto v = PickFloat(s, {"border-width"}))
        b.SetBorderWidth(*v);
    if (auto v = PickInt(s, {"font-size"}))
        b.SetFontSize(*v);
    if (auto v = PickString(s, {"font", "font-family", "font-name"}))
        b.SetFont(*v);
    if (auto v = PickFloat(s, {"radius", "border-radius", "corner-radius"}))
        b.SetCornerRadius(*v);
}

// ==========================================
// Panel
// ==========================================
inline void ApplyStyle(aur::Panel& p, const Style& s) {
    if (auto v = PickColor(s, {"bg", "background", "background-color"}))
        p.SetBgColor(*v);
    if (auto v = PickColor(s, {"border", "border-color"}))
        p.SetBorderColor(*v);
    if (auto v = PickColor(s, {"title-color", "heading-color"}))
        p.SetTitleColor(*v);
    if (auto v = PickFloat(s, {"border-width"}))
        p.SetBorderWidth(*v);
    if (auto v = PickInt(s, {"title-size", "title-font-size"}))
        p.SetTitleSize(*v);
    if (auto v = PickFloat(s, {"title-padding", "padding"}))
        p.SetTitlePadding(*v);
    if (auto v = PickFloat(s, {"radius", "border-radius", "corner-radius"}))
        p.SetCornerRadius(*v);
    if (auto v = PickString(s, {"font", "font-family", "font-name"}))
        p.SetFont(*v);
}

// ==========================================
// ProgressBar
// ==========================================
inline void ApplyStyle(aur::ProgressBar& pb, const Style& s) {
    if (auto v = PickColor(s, {"track", "track-color", "background", "bg"}))
        pb.SetTrackColor(*v);
    if (auto v = PickColor(s, {"fill", "fill-color", "foreground", "progress-color"}))
        pb.SetFillColor(*v);
    if (auto v = PickColor(s, {"border", "border-color"}))
        pb.SetBorderColor(*v);
    if (auto v = PickColor(s, {"text", "text-color", "color"}))
        pb.SetTextColor(*v);
    if (auto v = PickFloat(s, {"border-width"}))
        pb.SetBorderWidth(*v);
    if (auto v = PickFloat(s, {"radius", "border-radius", "corner-radius"}))
        pb.SetCornerRadius(*v);
    if (auto v = PickBool(s, {"show-percent", "show-label"}))
        pb.SetShowPercent(*v);
    if (auto v = PickInt(s, {"font-size"}))
        pb.SetFontSize(*v);
    if (auto v = PickString(s, {"font", "font-family", "font-name"}))
        pb.SetFont(*v);
}

// ==========================================
// TextBox
// ==========================================
inline void ApplyStyle(aur::TextBox& tb, const Style& s) {
    if (auto v = PickColor(s, {"bg", "background", "background-color"}))
        tb.SetBackgroundColor(*v);
    if (auto v = PickColor(s, {"border", "border-color"}))
        tb.SetBorderColor(*v);
    if (auto v = PickColor(s, {"focus-border", "focus-border-color",
                               "active-border", "active-border-color"}))
        tb.SetFocusBorderColor(*v);
    if (auto v = PickColor(s, {"text", "text-color", "color", "foreground"}))
        tb.SetTextColor(*v);
    if (auto v = PickColor(s, {"placeholder-color", "hint-color"}))
        tb.SetPlaceholderColor(*v);
    if (auto v = PickInt(s, {"font-size"}))
        tb.SetFontSize(*v);
    if (auto v = PickString(s, {"font", "font-family", "font-name"}))
        tb.SetFont(*v);
    if (auto v = PickInt(s, {"max-chars", "max-length"}))
        tb.SetMaxChars(*v);
}

// ==========================================
// Text
// ==========================================
inline void ApplyStyle(aur::Text& t, const Style& s) {
    if (auto v = PickColor(s, {"color", "text-color", "foreground"}))
        t.SetColor(*v);
    if (auto v = PickInt(s, {"font-size"}))
        t.SetFontSize(*v);
    if (auto v = PickFloat(s, {"spacing", "letter-spacing"}))
        t.SetSpacing(*v);
    if (auto v = PickString(s, {"font", "font-family", "font-name"}))
        t.SetFont(*v);
    if (auto v = PickBool(s, {"always-on-top", "on-top"}))
        t.SetAlwaysOnTop(*v);
}

// ==========================================
// Slider
// ==========================================
inline void ApplyStyle(aur::Slider& sl, const Style& s) {
    if (auto v = PickColor(s, {"track", "track-color"}))
        sl.SetTrackColor(*v);
    if (auto v = PickColor(s, {"fill", "fill-color", "progress-color"}))
        sl.SetFillColor(*v);
    if (auto v = PickColor(s, {"thumb", "thumb-color", "handle-color"}))
        sl.SetThumbColor(*v);
    if (auto v = PickColor(s, {"thumb-border", "thumb-border-color", "handle-border"}))
        sl.SetThumbBorderColor(*v);
    if (auto v = PickColor(s, {"label-color", "label"}))
        sl.SetLabelColor(*v);
    if (auto v = PickColor(s, {"value-color"}))
        sl.SetValueColor(*v);
    if (auto v = PickFloat(s, {"label-width"}))
        sl.SetLabelWidth(*v);
    if (auto v = PickString(s, {"font", "font-family", "font-name"}))
        sl.SetFont(*v);
}

// ==========================================
// Table
// ==========================================
inline void ApplyStyle(aur::Table& t, const Style& s) {
    if (auto v = PickColor(s, {"bg", "background", "background-color"}))
        t.SetBgColor(*v);
    if (auto v = PickColor(s, {"border", "border-color"}))
        t.SetBorderColor(*v);
    if (auto v = PickColor(s, {"divider", "divider-color", "separator"}))
        t.SetDividerColor(*v);
    if (auto v = PickColor(s, {"title-color", "heading-color"}))
        t.SetTitleColor(*v);
    if (auto v = PickColor(s, {"text", "text-color", "color", "foreground"}))
        t.SetTextColor(*v);
    if (auto v = PickFloat(s, {"padding", "cell-padding"}))
        t.SetPadding(*v);
    if (auto v = PickFloat(s, {"row-height", "cell-height", "line-height"}))
        t.SetCellRowHeight(*v);
    if (auto v = PickInt(s, {"title-size", "title-font-size"}))
        t.SetTitleFontSize(*v);
    if (auto v = PickInt(s, {"cell-size", "cell-font-size", "font-size"}))
        t.SetCellFontSize(*v);
    if (auto v = PickString(s, {"font", "font-family", "font-name"}))
        t.SetFont(*v);
}

// ==========================================
// Plot
// ==========================================
inline void ApplyStyle(aur::Plot& p, const Style& s) {
    if (auto v = PickColor(s, {"bg", "background", "background-color"}))
        p.SetBgColor(*v);
    if (auto v = PickColor(s, {"grid", "grid-color", "gridline"}))
        p.SetGridColor(*v);
    if (auto v = PickColor(s, {"border", "border-color"}))
        p.SetBorderColor(*v);
    if (auto v = PickColor(s, {"tick-color", "tick", "axis-tick"}))
        p.SetTickColor(*v);
    if (auto v = PickColor(s, {"axis-color", "axis-label-color", "label-color"}))
        p.SetAxisLabelColor(*v);
    if (auto v = PickInt(s, {"tick-size", "tick-font-size"}))
        p.SetTickFontSize(*v);
    if (auto v = PickInt(s, {"label-size", "label-font-size"}))
        p.SetLabelFontSize(*v);
    if (auto v = PickInt(s, {"y-divisions", "grid-rows"}))
        p.SetYDivisions(*v);
    if (auto v = PickInt(s, {"legend-size", "legend-font-size"}))
        p.SetLegendFontSize(*v);
    if (auto v = PickInt(s, {"legend-row", "legend-row-height"}))
        p.SetLegendRowHeight(*v);
    if (auto v = PickString(s, {"font", "font-family", "font-name"}))
        p.SetFont(*v);

    if (auto v = s.GetString("x-label")) p.SetXLabel(*v);
    if (auto v = s.GetString("y-label")) p.SetYLabel(*v);
    if (auto v = PickFloat(s, {"max-x", "x-max"}))       p.SetMaxX(*v);
    if (auto v = PickFloat(s, {"x-scale"}))              p.SetXScale(*v);
    if (auto v = PickBool (s, {"auto-y", "auto-y-scale"})) p.SetAutoYScale(*v);
}

// ==========================================
// 支持空格分隔的多个 selector
// "button primary" → 先 [button]，再 [primary]
// ==========================================
template<typename T>
inline void StyleSheet::Apply(T& component, const std::string& selector) const {
    std::istringstream iss(selector);
    std::string token;
    while (iss >> token) {
        auto* s = Find(token);
        if (s) ApplyStyle(component, *s);
    }
}

} // namespace aur::style
