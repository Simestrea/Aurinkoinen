#pragma once
#include "raylib.h"
#include "plot_axis.hpp"
#include "plot_legend.hpp"
#include "plot_series.hpp"
#include <aurinko/draw.hpp>
#include <string>
#include <vector>
#include <functional>

namespace aur {

// ==========================================
// Plot：折线图顶层类
//   - 组合 PlotAxis + PlotLegend + 一组 PlotSeries
//   - 对外提供链式 API
//   - Draw() 里按 背景 → 轴 → 曲线 → 图例 的顺序组装
// ==========================================
class Plot {
public:
    using LegendPos = PlotLegend::Pos;

    Plot(float x, float y, float w, float h)
        : x_{x}, y_{y}, w_{w}, h_{h} {}

    // 布局接口（供 aur::Layout 使用）
    void  SetPosition(float x, float y) { x_ = x; y_ = y; }
    void  SetSize(float w, float h)     { w_ = w; h_ = h; }
    float GetX() const { return x_; }
    float GetY() const { return y_; }
    float GetW() const { return w_; }
    float GetH() const { return h_; }

    // ==========================================
    // 数据
    // ==========================================
    Plot& AddSeries(const std::string& name, Color color,
                    const double* values, int maxPoints, double scale = 1.0) {
        PlotSeries s;
        s.name = name; s.color = color;
        s.values = values; s.maxPoints = maxPoints; s.scale = scale;
        series_.push_back(s);
        legend_.Invalidate();
        return *this;
    }
    Plot& ClearSeries() {
        series_.clear();
        legend_.Invalidate();
        return *this;
    }
    int SeriesCount() const { return (int)series_.size(); }

    // ==========================================
    // 轴
    // ==========================================
    Plot& SetMaxX(float m) {
        axis_.maxX_ = (m > 0.0f) ? m : 10.0f;
        axis_.Invalidate(); return *this;
    }
    Plot& SetXScale(float s) {
        axis_.xScale_ = (s > 0.0f) ? s : 1.0f;
        axis_.Invalidate(); return *this;
    }
    Plot& SetXTickStep(int n) {
        axis_.xTickStep_ = (n > 0) ? n : 0;
        axis_.Invalidate(); return *this;
    }
    Plot& SetYDivisions(int n) {
        axis_.yDivisions_ = (n > 0) ? n : 5;
        axis_.Invalidate(); return *this;
    }
    Plot& SetYRange(double mn, double mx) {
        axis_.yMin_ = mn;
        axis_.yMax_ = (mx > mn) ? mx : mn + 1.0;
        axis_.autoScale_ = false;
        return *this;
    }
    Plot& SetAutoYScale(bool b)  { axis_.autoScale_ = b; return *this; }
    Plot& SetYHeadroom(double h) { axis_.autoHeadroom_ = h; return *this; }
    Plot& SetXLabel(const std::string& s) {
        axis_.xLabel_ = s; axis_.Invalidate(); return *this;
    }
    Plot& SetYLabel(const std::string& s) {
        axis_.yLabel_ = s; axis_.Invalidate(); return *this;
    }
    Plot& SetLabels(const std::string& x, const std::string& y) {
        axis_.xLabel_ = x; axis_.yLabel_ = y;
        axis_.Invalidate(); return *this;
    }

    // ==========================================
    // 样式
    // ==========================================
    Plot& SetFont(const std::string& f) {
        axis_.fontName_ = f; axis_.Invalidate();
        legend_.fontName_ = f; legend_.Invalidate();
        return *this;
    }
    Plot& SetStyle(const std::string& f, int tickSize = 14, int labelSize = 16) {
        SetFont(f);
        SetTickFontSize(tickSize);
        SetLabelFontSize(labelSize);
        return *this;
    }
    Plot& SetTickFontSize(int s) {
        axis_.tickFontSize_ = s; axis_.Invalidate(); return *this;
    }
    Plot& SetLabelFontSize(int s) {
        axis_.labelFontSize_ = s; axis_.Invalidate(); return *this;
    }
    Plot& SetBgColor(Color c)        { bgColor_ = c; return *this; }
    Plot& SetGridColor(Color c)      { axis_.gridColor_ = c; return *this; }
    Plot& SetBorderColor(Color c)    { axis_.borderColor_ = c; return *this; }
    Plot& SetTickColor(Color c) {
        axis_.tickColor_ = c; axis_.Invalidate(); return *this;
    }
    Plot& SetAxisLabelColor(Color c) {
        axis_.axisLabelColor_ = c; axis_.Invalidate(); return *this;
    }

    // ==========================================
    // 图例
    // ==========================================
    Plot& SetLegend(LegendPos pos, int columns = 0) {
        legend_.pos_ = pos; legend_.columns_ = columns;
        legend_.Invalidate(); return *this;
    }
    Plot& SetLegendFontSize(int s) {
        legend_.fontSize_ = s; legend_.Invalidate(); return *this;
    }
    Plot& SetLegendRowHeight(int h) { legend_.rowHeight_ = h; return *this; }
    Plot& SetLegendTextColor(Color a, Color i) {
        legend_.activeColor_ = a; legend_.inactiveColor_ = i; return *this;
    }

    // ==========================================
    // 交互
    // ==========================================
    Plot& Select(int idx) { selected_ = idx; return *this; }
    int Selected() const { return selected_; }
    Plot& SetOnSelect(std::function<void(int)> fn) {
        onSelect_ = std::move(fn); return *this;
    }
    Plot& SetHighlightLastPoint(bool on) { highlightLast_ = on; return *this; }

    // ==========================================
    // 绘制
    // ==========================================
    void Draw() {
        // 1. 背景
        aur::draw::Rect(x_, y_, w_, h_, bgColor_);

        // 2. 自动 Y 缩放
        double dataMax = 0.0;
        for (auto& s : series_) {
            double m = plot_detail::MaxValue(s);
            if (m > dataMax) dataMax = m;
        }
        axis_.AutoFitY(dataMax);

        // 3. 轴
        axis_.Draw(x_, y_, w_, h_);

        // 4. 曲线
        for (int si = 0; si < (int)series_.size(); ++si) {
            auto& s = series_[si];
            bool active = (si == selected_);
            float thick = active ? 3.5f : 2.0f;
            Color cc = active ? s.color : aur::draw::Fade(s.color, 0.4f);

            auto xFn = [this](int k) {
                return axis_.XAt(axis_.XValueOf(k), x_, w_);
            };
            auto yFn = [this](double v) {
                return axis_.YAt(v, y_, h_);
            };

            plot_detail::DrawLine(s, x_, y_, w_, h_,
                                  xFn, yFn, thick, cc);

            if (active) {
                plot_detail::DrawPoints(s, x_, y_, w_, h_,
                                        xFn, yFn, highlightLast_, s.color);
            }
        }

        // 5. 图例
        std::vector<PlotLegend::Item> items;
        items.reserve(series_.size());
        for (auto& s : series_) items.push_back({ s.name, s.color });

        int newSel = legend_.Draw(
            items, x_, y_, w_, h_,
            axis_.tickFontSize_, axis_.labelFontSize_,
            !axis_.xLabel_.empty(),
            selected_);

        if (newSel != selected_) {
            selected_ = newSel;
            if (onSelect_) onSelect_(newSel);
        }
    }

private:
    float x_, y_, w_, h_;
    std::vector<PlotSeries> series_;
    PlotAxis   axis_;
    PlotLegend legend_;
    Color bgColor_{255, 255, 255, 255};

    int selected_ = -1;
    bool highlightLast_ = true;
    std::function<void(int)> onSelect_;
};

} // namespace aur