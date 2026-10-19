#pragma once
#include "raylib.h"
#include <aurinko/text.hpp>
#include <aurinko/draw.hpp>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>

namespace aur {

// ==========================================
// Table：只读字符串表格
//
// 特点：
//   - 可选标题，标题下自动画分隔线
//   - N 行 × M 列，列宽均分
//   - 单元格内容可每帧 SetCell 更新
//   - 单元格可单独改颜色
//
// 用法：
//   aur::Table table(60, 8, 700, 96);
//   table.SetFont(Globals::FONT_PATH)
//        .SetColumns(4)
//        .SetRows(3)
//        .SetTitle("数值：泥沙");
//
//   // 主循环里
//   table.SetCell(0, 0, "S= 1  w=0.8573  τ=0.050 ns");
//   table.SetCell(0, 1, "S= 2  w=0.6456  τ=0.073 ns");
//   // ...
//   table.Draw();
//
// 注意：
//   - Draw() 里会给每个 cell 设位置；文字由 DrawAllTexts 统一绘制
//   - SetCell 惰性创建 Text 对象，行列数变了会保留已有单元格
//   - 想清空所有单元格用 ClearCells()
// ==========================================
class Table {
private:
    float x_, y_, w_, h_;
    int rows_ = 0;
    int cols_ = 1;

    float padding_ = 10.0f;
    float cellRowHeight_ = 20.0f;
    int titleFontSize_ = 15;
    int cellFontSize_ = 13;

    std::string title_;
    std::string fontName_;

    Color bgColor_{255, 255, 255, 240};
    Color borderColor_{200, 200, 200, 255};
    Color dividerColor_{200, 200, 200, 255};
    Color titleColor_{2, 60, 120, 255};
    Color defaultTextColor_{50, 50, 50, 255};

    std::unique_ptr<Text> titleText_;
    std::vector<std::unique_ptr<Text>> cells_;

public:
    Table(float x, float y, float w, float h)
        : x_{x}, y_{y}, w_{w}, h_{h} {}

    // ==========================================
    // 布局
    // ==========================================
    Table& SetPosition(float x, float y) { x_ = x; y_ = y; return *this; }
    Table& SetSize(float w, float h) { w_ = w; h_ = h; return *this; }
    Table& SetPadding(float p) { padding_ = p; return *this; }
    Table& SetCellRowHeight(float h) { cellRowHeight_ = h; return *this; }
    Table& SetTitleFontSize(int s) {
        titleFontSize_ = s;
        titleText_.reset();
        return *this;
    }
    Table& SetCellFontSize(int s) { cellFontSize_ = s; return *this; }
    Table& SetRows(int n) { rows_ = std::max(0, n); return *this; }
    Table& SetColumns(int n) { cols_ = std::max(1, n); return *this; }

    // ==========================================
    // 样式
    // ==========================================
    Table& SetFont(const std::string& f) {
        fontName_ = f;
        titleText_.reset();
        return *this;
    }
    Table& SetBgColor(Color c)        { bgColor_ = c; return *this; }
    Table& SetBorderColor(Color c)    { borderColor_ = c; return *this; }
    Table& SetDividerColor(Color c)   { dividerColor_ = c; return *this; }
    Table& SetTitleColor(Color c) {
        titleColor_ = c;
        if (titleText_) titleText_->SetColor(c);
        return *this;
    }
    Table& SetTextColor(Color c)      { defaultTextColor_ = c; return *this; }

    // ==========================================
    // 内容
    // ==========================================
    Table& SetTitle(const std::string& s) {
        title_ = s;
        if (!titleText_) {
            titleText_ = std::make_unique<Text>(
                title_, 0.0f, 0.0f, titleFontSize_,
                titleColor_, fontName_);
        } else {
            titleText_->SetText(title_);
        }
        return *this;
    }

    Table& SetCell(int row, int col, const std::string& text) {
        if (row < 0 || col < 0 || col >= cols_) return *this;
        int idx = row * cols_ + col;
        if (idx >= (int)cells_.size()) cells_.resize(idx + 1);
        if (!cells_[idx]) {
            cells_[idx] = std::make_unique<Text>(
                text, 0.0f, 0.0f, cellFontSize_,
                defaultTextColor_, fontName_);
        } else {
            cells_[idx]->SetText(text);
        }
        return *this;
    }

    Table& SetCellColor(int row, int col, Color c) {
        if (row < 0 || col < 0 || col >= cols_) return *this;
        int idx = row * cols_ + col;
        if (idx < (int)cells_.size() && cells_[idx])
            cells_[idx]->SetColor(c);
        return *this;
    }

    Table& SetAllCellColors(Color c) {
        for (auto& cell : cells_)
            if (cell) cell->SetColor(c);
        return *this;
    }

    Table& ClearCells() { cells_.clear(); return *this; }

    // ==========================================
    // 绘制
    // ==========================================
    void Draw() {
        aur::draw::Rect(x_, y_, w_, h_, bgColor_);
        aur::draw::RectOutline(x_, y_, w_, h_, borderColor_);

        float contentTop = y_ + padding_;

        if (!title_.empty() && titleText_) {
            titleText_->SetPosition(x_ + padding_, contentTop);
            float dividerY = contentTop + titleFontSize_ + 6.0f;
            aur::draw::Line(x_ + padding_, dividerY,
                            x_ + w_ - padding_, dividerY, dividerColor_);
            contentTop = dividerY + 6.0f;
        }

        float colWidth = (w_ - 2.0f * padding_) / (float)cols_;
        for (int i = 0; i < (int)cells_.size(); ++i) {
            if (!cells_[i]) continue;
            int row = i / cols_;
            int col = i % cols_;
            float cx = x_ + padding_ + col * colWidth;
            float cy = contentTop + row * cellRowHeight_;
            cells_[i]->SetPosition(cx, cy);
        }
    }
};

} // namespace aur