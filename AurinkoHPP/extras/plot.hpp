#pragma once
#include "raylib.h"
#include <aurinko/init.hpp>
#include <aurinko/text.hpp>
#include <aurinko/draw.hpp>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <functional>
#include <cmath>
#include <cstdio>

namespace aur {

class Plot {
public:
    enum class LegendPos { None, Bottom, Top, Right };

private:
    struct Series {
        std::string name;
        Color color;
        const double* values = nullptr;
        int maxPoints = 0;
        double scale = 1.0;
    };

    float x_, y_, w_, h_;
    std::vector<Series> series_;

    double yMin_ = 0.0;
    double yMax_ = 1.0;
    bool autoScale_ = true;
    double autoHeadroom_ = 1.15;
    int yDivisions_ = 5;
    int maxX_ = 10;

    std::string xLabel_, yLabel_;

    Color bgColor_{255, 255, 255, 255};
    Color gridColor_{230, 230, 230, 255};
    Color borderColor_{180, 180, 180, 255};
    Color tickColor_{80, 80, 80, 255};
    Color axisLabelColor_{80, 80, 80, 255};

    std::string fontName_;
    int tickFontSize_ = 14;
    int labelFontSize_ = 16;

    LegendPos legendPos_ = LegendPos::None;
    int legendColumns_ = 0;
    int legendFontSize_ = 14;
    int legendRowHeight_ = 24;
    Color legendActiveColor_{BLACK};
    Color legendInactiveColor_{GRAY};

    bool highlightLastPoint_ = true;

    int selected_ = 0;
    std::function<void(int)> onSelect_;

    std::vector<std::unique_ptr<Text>> xTickTexts_;
    std::vector<std::unique_ptr<Text>> yTickTexts_;
    std::unique_ptr<Text> xLabelText_, yLabelText_;
    std::vector<std::unique_ptr<Text>> legendTexts_;
    bool textInitialized_ = false;

    void InvalidateText() { textInitialized_ = false; }

    void EnsureText() {
        if (textInitialized_) return;

        xTickTexts_.clear();
        yTickTexts_.clear();
        xLabelText_.reset();
        yLabelText_.reset();
        legendTexts_.clear();

        for (int i = 0; i <= maxX_; ++i)
            xTickTexts_.push_back(std::make_unique<Text>(
                "0", 0.0f, 0.0f, tickFontSize_, tickColor_, fontName_));
        for (int i = 0; i <= yDivisions_; ++i)
            yTickTexts_.push_back(std::make_unique<Text>(
                "0.00", 0.0f, 0.0f, tickFontSize_, tickColor_, fontName_));
        if (!xLabel_.empty())
            xLabelText_ = std::make_unique<Text>(
                xLabel_, 0.0f, 0.0f, labelFontSize_, axisLabelColor_, fontName_);
        if (!yLabel_.empty())
            yLabelText_ = std::make_unique<Text>(
                yLabel_, 0.0f, 0.0f, labelFontSize_, axisLabelColor_, fontName_);
        // for (auto& s : series_)
        //     legendTexts_.push_back(std::make_unique<Text>(
        //         s.name, 0.0f, 0.0f, legendFontSize_,
        //         legendActiveColor_, fontName_));
        if (legendPos_ != LegendPos::None) {
            for (auto& s : series_)
                legendTexts_.push_back(std::make_unique<Text>(
                    s.name, 0.0f, 0.0f, legendFontSize_,
                    legendActiveColor_, fontName_));
        }

        textInitialized_ = true;
    }

    float xAt(float k) const {
        if (maxX_ <= 0) return x_;
        return x_ + w_ * k / (float)maxX_;
    }

    float yAt(double v) const {
        if (yMax_ <= yMin_) return y_ + h_;
        double t = (v - yMin_) / (yMax_ - yMin_);
        return y_ + h_ - h_ * (float)t;
    }

    static bool InvalidValue(double v) {
        return std::isnan(v) || v == 0.0;
    }

    double ScaledValue(const Series& s, int k) const {
        return s.values[k] * s.scale;
    }

    int FindLastValid(const Series& s) const {
        if (!s.values) return -1;
        for (int k = s.maxPoints - 1; k >= 1; --k) {
            if (!InvalidValue(ScaledValue(s, k))) return k;
        }
        return -1;
    }

    void ComputeAutoScale() {
        if (!autoScale_) return;
        double m = 1e-30;
        for (auto& s : series_) {
            if (!s.values) continue;
            for (int i = 1; i < s.maxPoints; ++i) {
                double v = ScaledValue(s, i);
                if (InvalidValue(v)) continue;
                if (v > m) m = v;
            }
        }
        m *= autoHeadroom_;
        if (m < 1e-12) m = 1e-6;
        yMax_ = m;
        yMin_ = 0.0;
    }

    struct LegendItem { Rectangle rect; int index; };

    std::vector<LegendItem> ComputeLegendLayout() const {
        std::vector<LegendItem> result;
        if (legendPos_ == LegendPos::None) return result;
        if (series_.empty()) return result;

        int n = (int)series_.size();
        int cols = legendColumns_;
        if (cols <= 0) {
            if (legendPos_ == LegendPos::Right) cols = 1;
            else {
                cols = std::max(1, (int)(w_ / 120.0f));
                cols = std::min(cols, n);
            }
        }
        int rows = (n + cols - 1) / cols;
        float itemW = (legendPos_ == LegendPos::Right) ? 140.0f
                                                       : (w_ / cols);
        float itemH = (float)legendRowHeight_;

        float lx = x_, ly = y_;
        if (legendPos_ == LegendPos::Bottom) {
            float yOff;
            if (!xLabel_.empty())
                yOff = 6.0f + tickFontSize_ + 8.0f + labelFontSize_ + 12.0f;
            else
                yOff = 6.0f + tickFontSize_ + 14.0f;
            ly = y_ + h_ + yOff;
        } else if (legendPos_ == LegendPos::Top) {
            ly = y_ - rows * itemH - 8.0f;
        } else if (legendPos_ == LegendPos::Right) {
            lx = x_ + w_ + 12.0f;
        }

        for (int i = 0; i < n; ++i) {
            int row = i / cols;
            int col = i % cols;
            float ix = lx + col * itemW;
            float iy = ly + row * itemH;
            result.push_back({ { ix, iy, itemW - 4.0f, itemH }, i });
        }
        return result;
    }

    void DrawLegend() {
        if (legendPos_ == LegendPos::None) return;
        if (series_.empty()) return;

        auto items = ComputeLegendLayout();
        Vector2 mouse = detail::virtualScreen.GetVirtualMousePosition();

        for (auto& item : items) {
            int i = item.index;
            Rectangle r = item.rect;

            bool hover = aur::draw::PointInRect(mouse, r);
            if (hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                selected_ = i;
                if (onSelect_) onSelect_(i);
            }

            bool active = (i == selected_);

            float sx = r.x + 4;
            float sy = r.y + r.height * 0.5f;
            aur::draw::Rect(sx, sy - 3, 16, 6,
                            active ? series_[i].color
                                   : aur::draw::Fade(series_[i].color, 0.5f));
            aur::draw::Circle(sx + 8, sy, active ? 5 : 3, series_[i].color);

            if (i < (int)legendTexts_.size()) {
                legendTexts_[i]->SetPosition(
                    sx + 24,
                    r.y + (r.height - legendFontSize_) * 0.5f);
                legendTexts_[i]->SetColor(active ? legendActiveColor_
                                                 : legendInactiveColor_);
            }
        }
    }

public:
    Plot(float x, float y, float w, float h)
        : x_{x}, y_{y}, w_{w}, h_{h} {}

    // ==========================================
    // 数据
    // ==========================================
    Plot& AddSeries(const std::string& name, Color color,
                    const double* values, int maxPoints, double scale = 1.0) {
        Series s;
        s.name = name;
        s.color = color;
        s.values = values;
        s.maxPoints = maxPoints;
        s.scale = scale;
        series_.push_back(s);
        InvalidateText();
        return *this;
    }

    Plot& ClearSeries() {
        series_.clear();
        InvalidateText();
        return *this;
    }

    int SeriesCount() const { return (int)series_.size(); }

    // ==========================================
    // 轴
    // ==========================================
    Plot& SetYRange(double minV, double maxV) {
        yMin_ = minV;
        yMax_ = (maxV > minV) ? maxV : minV + 1.0;
        autoScale_ = false;
        return *this;
    }
    Plot& SetAutoYScale(bool b) { autoScale_ = b; return *this; }
    Plot& SetYHeadroom(double h) { autoHeadroom_ = h; return *this; }
    Plot& SetMaxX(int m) {
        maxX_ = (m > 0) ? m : 10;
        InvalidateText();
        return *this;
    }
    Plot& SetYDivisions(int n) {
        yDivisions_ = (n > 0) ? n : 5;
        InvalidateText();
        return *this;
    }
    Plot& SetXLabel(const std::string& s) {
        if (xLabel_ == s) return *this;
        xLabel_ = s; InvalidateText(); return *this;
    }
    Plot& SetYLabel(const std::string& s) {
        if (yLabel_ == s) return *this;
        yLabel_ = s; InvalidateText(); return *this;
    }
    Plot& SetLabels(const std::string& xLab, const std::string& yLab) {
        SetXLabel(xLab); SetYLabel(yLab); return *this;
    }

    // ==========================================
    // 样式
    // ==========================================
    Plot& SetFont(const std::string& f) {
        if (fontName_ == f) return *this;
        fontName_ = f; InvalidateText(); return *this;
    }
    Plot& SetStyle(const std::string& font,
                   int tickSize = 14, int labelSize = 16) {
        SetFont(font);
        SetTickFontSize(tickSize);
        SetLabelFontSize(labelSize);
        return *this;
    }
    Plot& SetTickFontSize(int s) {
        if (tickFontSize_ == s) return *this;
        tickFontSize_ = s; InvalidateText(); return *this;
    }
    Plot& SetLabelFontSize(int s) {
        if (labelFontSize_ == s) return *this;
        labelFontSize_ = s; InvalidateText(); return *this;
    }
    Plot& SetBgColor(Color c)        { bgColor_ = c; return *this; }
    Plot& SetGridColor(Color c)      { gridColor_ = c; return *this; }
    Plot& SetBorderColor(Color c)    { borderColor_ = c; return *this; }
    Plot& SetTickColor(Color c)      { tickColor_ = c; return *this; }
    Plot& SetAxisLabelColor(Color c) { axisLabelColor_ = c; return *this; }

    // ==========================================
    // 图例
    // ==========================================
    // Plot& SetLegend(LegendPos pos, int columns = 0) {
    //     legendPos_ = pos;
    //     legendColumns_ = columns;
    //     return *this;
    // }
    Plot& SetLegend(LegendPos pos, int columns = 0) {
        legendPos_ = pos;
        legendColumns_ = columns;
        InvalidateText();
        return *this;
    }
    Plot& SetLegendFontSize(int s) {
        legendFontSize_ = s; InvalidateText(); return *this;
    }
    Plot& SetLegendRowHeight(int h) { legendRowHeight_ = h; return *this; }
    Plot& SetLegendTextColor(Color active, Color inactive) {
        legendActiveColor_ = active;
        legendInactiveColor_ = inactive;
        return *this;
    }

    // ==========================================
    // 交互
    // ==========================================
    Plot& Select(int idx) {
        selected_ = idx;
        return *this;
    }
    int Selected() const { return selected_; }
    Plot& SetOnSelect(std::function<void(int)> fn) {
        onSelect_ = std::move(fn);
        return *this;
    }
    Plot& SetHighlightLastPoint(bool on) {
        highlightLastPoint_ = on;
        return *this;
    }

    // ==========================================
    // 绘制
    // ==========================================
    void Draw() {
        EnsureText();
        ComputeAutoScale();

        aur::draw::Rect(x_, y_, w_, h_, bgColor_);

        for (int i = 0; i <= maxX_; ++i) {
            float xx = xAt((float)i);
            aur::draw::Line(xx, y_, xx, y_ + h_, gridColor_);
        }
        for (int i = 0; i <= yDivisions_; ++i) {
            float yy = y_ + h_ - h_ * i / (float)yDivisions_;
            aur::draw::Line(x_, yy, x_ + w_, yy, gridColor_);
        }

        aur::draw::RectOutline(x_, y_, w_, h_, borderColor_);

        for (int i = 0; i <= maxX_ && i < (int)xTickTexts_.size(); ++i) {
            char buf[8];
            snprintf(buf, sizeof(buf), "%d", i);
            xTickTexts_[i]->SetText(buf);
            float tw = xTickTexts_[i]->MeasureWidth();
            xTickTexts_[i]->SetPosition(xAt((float)i) - tw * 0.5f,
                                         y_ + h_ + 6.0f);
        }

        for (int i = 0; i <= yDivisions_ && i < (int)yTickTexts_.size(); ++i) {
            double v = yMin_ + (yMax_ - yMin_) * i / (double)yDivisions_;
            char buf[16];
            snprintf(buf, sizeof(buf), "%.2f", v);
            yTickTexts_[i]->SetText(buf);
            float tw = yTickTexts_[i]->MeasureWidth();
            float ty = y_ + h_ - h_ * i / (float)yDivisions_;
            yTickTexts_[i]->SetPosition(x_ - tw - 8.0f,
                                         ty - tickFontSize_ * 0.5f);
        }

        if (xLabelText_) {
            float tw = xLabelText_->MeasureWidth();
            xLabelText_->SetPosition(
                x_ + (w_ - tw) * 0.5f,
                y_ + h_ + 6.0f + tickFontSize_ + 8.0f);
        }
        if (yLabelText_) {
            yLabelText_->SetPosition(x_, y_ - labelFontSize_ - 8.0f);
        }

        for (int si = 0; si < (int)series_.size(); ++si) {
            auto& s = series_[si];
            if (!s.values) continue;

            int lastValid = FindLastValid(s);
            if (lastValid < 1) continue;

            bool active = (si == selected_);
            float thickness = active ? 3.5f : 2.0f;
            Color cc = active ? s.color : aur::draw::Fade(s.color, 0.4f);

            int prev = -1;
            for (int k = 1; k <= lastValid; ++k) {
                double v = ScaledValue(s, k);
                if (InvalidValue(v)) { prev = -1; continue; }

                if (prev >= 0) {
                    aur::draw::LineThick(
                        xAt((float)prev), yAt(ScaledValue(s, prev)),
                        xAt((float)k),    yAt(v),
                        thickness, cc);
                }
                prev = k;
            }

            for (int k = 1; k <= lastValid; ++k) {
                double v = ScaledValue(s, k);
                if (InvalidValue(v)) continue;
                float px = xAt((float)k);
                float py = yAt(v);
                bool hl = highlightLastPoint_ && (k == lastValid);
                aur::draw::Circle(px, py,
                    hl ? 7.0f : (active ? 5.0f : 3.0f), s.color);
                if (hl) aur::draw::CircleOutline(px, py, 11.0f, s.color);
            }
        }

        DrawLegend();
    }
};

} // namespace aur