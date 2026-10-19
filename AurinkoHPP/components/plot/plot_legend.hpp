#pragma once
#include "raylib.h"
#include <aurinko/init.hpp>
#include <aurinko/components/text.hpp>
#include <aurinko/draw.hpp>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>

namespace aur {

// ==========================================
// PlotLegend：图例
//   - 保存位置、列数、字号、颜色
//   - 计算每项的矩形
//   - 处理点击切换选中
//   - 绘制每一项（色块 + 点 + 文字）
//   - 不关心曲线数据怎么画
// ==========================================
class PlotLegend {
public:
    enum class Pos { None, Bottom, Top, Right };

    struct Item {
        std::string name;
        Color color;
    };

    Pos pos_ = Pos::None;
    int columns_ = 0;
    int fontSize_ = 14;
    int rowHeight_ = 24;
    Color activeColor_{BLACK};
    Color inactiveColor_{GRAY};
    std::string fontName_;

    // Text 缓存
    std::vector<std::unique_ptr<Text>> texts_;
    bool textInitialized_ = false;

    void Invalidate() { textInitialized_ = false; }

    struct LayoutItem { Rectangle rect; int index; };

    std::vector<LayoutItem> ComputeLayout(
        float px, float py, float pw, float ph,
        int itemCount,
        int tickFontSize, int labelFontSize,
        bool hasXLabel) const
    {
        std::vector<LayoutItem> result;
        if (pos_ == Pos::None) return result;
        if (itemCount <= 0) return result;

        int cols = columns_;
        if (cols <= 0) {
            if (pos_ == Pos::Right) cols = 1;
            else {
                cols = std::max(1, (int)(pw / 120.0f));
                cols = std::min(cols, itemCount);
            }
        }
        int rows = (itemCount + cols - 1) / cols;
        float itemW = (pos_ == Pos::Right) ? 140.0f : (pw / cols);
        float itemH = (float)rowHeight_;

        float lx = px, ly = py;
        if (pos_ == Pos::Bottom) {
            float yOff = hasXLabel
                ? 6.0f + tickFontSize + 8.0f + labelFontSize + 12.0f
                : 6.0f + tickFontSize + 14.0f;
            ly = py + ph + yOff;
        } else if (pos_ == Pos::Top) {
            ly = py - rows * itemH - 8.0f;
        } else if (pos_ == Pos::Right) {
            lx = px + pw + 12.0f;
        }

        for (int i = 0; i < itemCount; ++i) {
            int row = i / cols, col = i % cols;
            result.push_back({ { lx + col * itemW, ly + row * itemH,
                                 itemW - 4.0f, itemH }, i });
        }
        return result;
    }

    // 绘制 + 处理点击；返回新的选中索引（-1 = 未选中）
    int Draw(const std::vector<Item>& items,
             float px, float py, float pw, float ph,
             int tickFontSize, int labelFontSize, bool hasXLabel,
             int selected)
    {
        if (pos_ == Pos::None) return selected;
        if (items.empty()) return selected;

        EnsureTexts(items);

        auto layout = ComputeLayout(px, py, pw, ph, (int)items.size(),
                                    tickFontSize, labelFontSize, hasXLabel);
        Vector2 mouse = detail::virtualScreen.GetVirtualMousePosition();
        int newSelected = selected;

        for (auto& item : layout) {
            int i = item.index;
            Rectangle r = item.rect;

            bool hover = aur::draw::PointInRect(mouse, r);
            if (hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                newSelected = (selected == i) ? -1 : i;
            }

            bool active = (i == selected);
            float sx = r.x + 4;
            float sy = r.y + r.height * 0.5f;

            aur::draw::Rect(sx, sy - 3, 16, 6,
                            active ? items[i].color
                                   : aur::draw::Fade(items[i].color, 0.5f));
            aur::draw::Circle(sx + 8, sy, active ? 5 : 3, items[i].color);

            if (i < (int)texts_.size()) {
                texts_[i]->SetPosition(
                    sx + 24,
                    r.y + (r.height - fontSize_) * 0.5f);
                texts_[i]->SetColor(active ? activeColor_ : inactiveColor_);
            }
        }

        return newSelected;
    }

private:
    void EnsureTexts(const std::vector<Item>& items) {
        if (textInitialized_ && texts_.size() == items.size()) return;
        texts_.clear();
        for (auto& it : items)
            texts_.push_back(std::make_unique<Text>(
                it.name, 0.0f, 0.0f, fontSize_, activeColor_, fontName_));
        textInitialized_ = true;
    }
};

} // namespace aur