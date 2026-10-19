#pragma once
#include "raylib.h"
#include <aurinko/draw.hpp>
#include <string>
#include <cmath>

namespace aur {

// ==========================================
// PlotSeries：一条曲线
//   - 保存名字、颜色、数据指针
//   - 折线绘制（含 Liang-Barsky 裁剪）
//   - 数据点绘制（只画在图表矩形内）
//
// 不依赖 PlotAxis / PlotLegend：
//   坐标换算由调用方通过 lambda 传入（xFn(k) / yFn(v)）
// ==========================================
struct PlotSeries {
    std::string name;
    Color color;
    const double* values = nullptr;
    int maxPoints = 0;
    double scale = 1.0;
};

namespace plot_detail {

inline bool InvalidValue(double v) {
    return std::isnan(v) || v == 0.0;
}

// Liang-Barsky 裁剪，只画 [px,py,pw,ph] 内的部分
inline void DrawClippedSegment(float x0, float y0, float x1, float y1,
                               float px, float py, float pw, float ph,
                               float thickness, Color c)
{
    float t0 = 0.0f, t1 = 1.0f;
    float dx = x1 - x0, dy = y1 - y0;
    auto clip = [&](float p, float q) -> bool {
        if (p == 0.0f) return q >= 0.0f;
        float r = q / p;
        if (p < 0.0f) { if (r > t1) return false; if (r > t0) t0 = r; }
        else          { if (r < t0) return false; if (r < t1) t1 = r; }
        return true;
    };
    if (!clip(-dx, x0 - px))       return;
    if (!clip( dx, px + pw - x0))  return;
    if (!clip(-dy, y0 - py))       return;
    if (!clip( dy, py + ph - y0))  return;

    aur::draw::LineThick(
        x0 + t0 * dx, y0 + t0 * dy,
        x0 + t1 * dx, y0 + t1 * dy,
        thickness, c);
}

inline int FindLastValid(const PlotSeries& s) {
    if (!s.values) return -1;
    for (int k = s.maxPoints - 1; k >= 1; --k)
        if (!InvalidValue(s.values[k] * s.scale)) return k;
    return -1;
}

inline double MaxValue(const PlotSeries& s) {
    if (!s.values) return 0.0;
    double m = 0.0;
    for (int i = 1; i < s.maxPoints; ++i) {
        double v = s.values[i] * s.scale;
        if (InvalidValue(v)) continue;
        if (v > m) m = v;
    }
    return m;
}

// 画折线（含裁剪）。
// xFn(int k)     → 屏幕 X 坐标
// yFn(double v)  → 屏幕 Y 坐标
template <typename FX, typename FY>
void DrawLine(const PlotSeries& s,
              float px, float py, float pw, float ph,
              FX xFn, FY yFn,
              float thickness, Color c)
{
    if (!s.values) return;
    int lastValid = FindLastValid(s);
    if (lastValid < 1) return;

    int prev = -1;
    for (int k = 1; k <= lastValid; ++k) {
        double v = s.values[k] * s.scale;
        if (InvalidValue(v)) { prev = -1; continue; }
        if (prev >= 0) {
            double pv = s.values[prev] * s.scale;
            DrawClippedSegment(
                xFn(prev), yFn(pv),
                xFn(k),    yFn(v),
                px, py, pw, ph,
                thickness, c);
        }
        prev = k;
    }
}

// 画数据点（只画在矩形内，末点可高亮）
template <typename FX, typename FY>
void DrawPoints(const PlotSeries& s,
                float px, float py, float pw, float ph,
                FX xFn, FY yFn,
                bool highlightLast, Color c)
{
    if (!s.values) return;
    int lastValid = FindLastValid(s);
    if (lastValid < 1) return;

    for (int k = 1; k <= lastValid; ++k) {
        double v = s.values[k] * s.scale;
        if (InvalidValue(v)) continue;
        float sx = xFn(k);
        float sy = yFn(v);
        if (sx < px || sx > px + pw) continue;
        if (sy < py || sy > py + ph) continue;
        bool hl = highlightLast && (k == lastValid);
        aur::draw::Circle(sx, sy, hl ? 7.0f : 5.0f, c);
        if (hl) aur::draw::CircleOutline(sx, sy, 11.0f, c);
    }
}

} // namespace plot_detail

} // namespace aur