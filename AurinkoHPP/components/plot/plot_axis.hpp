#pragma once
#include "raylib.h"
#include <aurinko/components/text.hpp>
#include <aurinko/draw.hpp>
#include <string>
#include <vector>
#include <memory>
#include <cmath>
#include <cstdio>


namespace aur {

// ==========================================
// PlotAxis：坐标轴
//   - 保存 X 范围、Y 范围、刻度设置
//   - 坐标换算（数据 → 屏幕像素）
//   - 绘制网格、刻度、轴标题
//   - 不关心曲线数据，也不关心图例
// ==========================================
class PlotAxis {
public:
    // ---- X 轴 ----
    float maxX_ = 10.0f;
    float xScale_ = 1.0f;        // 每个数据索引对应多少 X 单位
    int   xTickStep_ = 0;        // 0 = 自动
    std::string xLabel_;

    // ---- Y 轴 ----
    double yMin_ = 0.0;
    double yMax_ = 1.0;
    bool   autoScale_ = true;
    double autoHeadroom_ = 1.15;
    int    yDivisions_ = 5;
    std::string yLabel_;

    // ---- 颜色 ----
    Color gridColor_{230, 230, 230, 255};
    Color borderColor_{180, 180, 180, 255};
    Color tickColor_{80, 80, 80, 255};
    Color axisLabelColor_{80, 80, 80, 255};

    // ---- 字体 ----
    std::string fontName_;
    int tickFontSize_ = 14;
    int labelFontSize_ = 16;

    // ---- Text 缓存 ----
    std::vector<std::unique_ptr<Text>> xTickTexts_;
    std::vector<std::unique_ptr<Text>> yTickTexts_;
    std::unique_ptr<Text> xLabelText_;
    std::unique_ptr<Text> yLabelText_;
    bool textInitialized_ = false;
    int  cachedXTickStep_ = -1;
    int  cachedYDivisions_ = -1;

    void Invalidate() { textInitialized_ = false; }

    // ---- 数据索引 → X 值 ----
    float XValueOf(int k) const { return (float)k * xScale_; }

    // ---- 坐标换算（需要 plot 矩形参数）----
    float XAt(float xValue, float px, float pw) const {
        if (maxX_ <= 0.0f) return px;
        return px + pw * xValue / maxX_;
    }
    float YAt(double v, float py, float ph) const {
        if (yMax_ <= yMin_) return py + ph;
        double t = (v - yMin_) / (yMax_ - yMin_);
        return py + ph - ph * (float)t;
    }

    // ---- X 轴刻度步长（自动或手动）----
    int EffectiveXTickStep(float plotW) const {
        if (xTickStep_ > 0) return xTickStep_;
        if (maxX_ <= 0.0f || plotW <= 0.0f) return 1;
        float pxPerUnit = plotW / maxX_;
        if (pxPerUnit <= 0.0f) return 1;
        int maxInt = (int)std::ceil(maxX_);
        int digits = 1;
        if (maxInt > 0) digits = (int)std::to_string(maxInt).size();
        float labelW = tickFontSize_ * 0.62f * (float)digits + 14.0f;
        int step = (int)std::ceil(labelW / pxPerUnit);
        return step > 1 ? step : 1;
    }

    // ---- 数据最大值 → 自动 Y 上限 ----
    void AutoFitY(double dataMax) {
        if (!autoScale_) return;
        double m = dataMax * autoHeadroom_;
        if (m < 1e-12) m = 1e-6;
        yMax_ = m;
        yMin_ = 0.0;
    }

    // ---- 绘制（网格 + 刻度 + 轴标题）----
    void Draw(float px, float py, float pw, float ph) {
        EnsureTexts(pw);
        DrawGrid(px, py, pw, ph);
        DrawRectangleLinesEx({ px, py, pw, ph }, 1.0f, borderColor_);
        DrawXTicks(px, py, pw, ph);
        DrawYTicks(px, py, pw, ph);
        DrawLabels(px, py, pw, ph);
    }

private:
    void EnsureTexts(float plotW) {
        int xStep = EffectiveXTickStep(plotW);
        if (textInitialized_
            && cachedXTickStep_ == xStep
            && cachedYDivisions_ == yDivisions_) return;

        xTickTexts_.clear();
        yTickTexts_.clear();
        xLabelText_.reset();
        yLabelText_.reset();

        int numXTicks = (int)std::floor(maxX_ / (float)xStep) + 1;
        for (int i = 0; i < numXTicks; ++i)
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

        cachedXTickStep_ = xStep;
        cachedYDivisions_ = yDivisions_;
        textInitialized_ = true;
    }

    void DrawGrid(float px, float py, float pw, float ph) {
        int xStep = EffectiveXTickStep(pw);
        for (float xv = 0.0f; xv <= maxX_ + 1e-6f; xv += (float)xStep) {
            float xx = XAt(xv, px, pw);
            aur::draw::Line(xx, py, xx, py + ph, gridColor_);
        }
        for (int i = 0; i <= yDivisions_; ++i) {
            float yy = py + ph - ph * i / (float)yDivisions_;
            aur::draw::Line(px, yy, px + pw, yy, gridColor_);
        }
    }

    void DrawXTicks(float px, float py, float pw, float ph) {
        int xStep = EffectiveXTickStep(pw);
        bool allInt = (std::floor(maxX_) == maxX_) && (xScale_ == 1.0f);
        int idx = 0;
        for (float xv = 0.0f;
             xv <= maxX_ + 1e-6f && idx < (int)xTickTexts_.size();
             xv += (float)xStep, ++idx) {
            char buf[32];
            if (allInt) {
                snprintf(buf, sizeof(buf), "%d", (int)std::round(xv));
            } else {
                float stepSize = (float)xStep * xScale_;
                if (stepSize >= 1.0f)       snprintf(buf, sizeof(buf), "%.0f", xv);
                else if (stepSize >= 0.1f)  snprintf(buf, sizeof(buf), "%.1f", xv);
                else if (stepSize >= 0.01f) snprintf(buf, sizeof(buf), "%.2f", xv);
                else                        snprintf(buf, sizeof(buf), "%.3f", xv);
            }
            xTickTexts_[idx]->SetText(buf);
            float tw = xTickTexts_[idx]->MeasureWidth();
            xTickTexts_[idx]->SetPosition(XAt(xv, px, pw) - tw * 0.5f,
                                           py + ph + 6.0f);
        }
    }

    void DrawYTicks(float px, float py, float pw, float ph) {
        for (int i = 0; i <= yDivisions_ && i < (int)yTickTexts_.size(); ++i) {
            double v = yMin_ + (yMax_ - yMin_) * i / (double)yDivisions_;
            char buf[32];
            snprintf(buf, sizeof(buf), "%.2f", v);
            yTickTexts_[i]->SetText(buf);
            float tw = yTickTexts_[i]->MeasureWidth();
            float ty = py + ph - ph * i / (float)yDivisions_;
            yTickTexts_[i]->SetPosition(px - tw - 8.0f,
                                         ty - tickFontSize_ * 0.5f);
        }
    }

    void DrawLabels(float px, float py, float pw, float ph) {
        if (xLabelText_) {
            float tw = xLabelText_->MeasureWidth();
            xLabelText_->SetPosition(
                px + (pw - tw) * 0.5f,
                py + ph + 6.0f + tickFontSize_ + 8.0f);
        }
        if (yLabelText_) {
            yLabelText_->SetPosition(px, py - labelFontSize_ - 8.0f);
        }
    }
};

} // namespace aur