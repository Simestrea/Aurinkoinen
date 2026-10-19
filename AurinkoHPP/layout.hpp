#pragma once
// ==========================================
// Layout: 极简垂直布局器
//
// 目的：消除手算 Y 坐标的重复劳动。
//
// 用法：
//   aur::Layout L(inputX, rowY0);
//   L.Gap(10);
//   L.Row(tbNphotons, 38);
//   L.Row(tbW0, 38);
//   L.Row(tbThetaMax, 38);
//   L.Skip(20);              // 空一段
//   L.Row(btnStart, 40);
//
//   // 换列
//   L.NewColumn(rightX, rowY0);
//   L.Row(panel, 300);
//
// 说明：
//   - Row() 调用组件的 SetPosition(x, y)
//   - 每个组件都有 SetPosition（Button / Panel / Text / TextBox /
//     Slider / Table / ProgressBar / Plot）
//   - 高度由调用者传（因为不同组件的"自然高度"语义不同）
// ==========================================

namespace aur {

class Layout {
private:
    float x_ = 0.0f;
    float y_ = 0.0f;
    float gap_ = 8.0f;

public:
    Layout() = default;

    Layout(float x, float y, float gap = 8.0f)
        : x_{x}, y_{y}, gap_{gap} {}

    // ---------- 配置 ----------
    Layout& Gap(float g)   { gap_ = g; return *this; }
    Layout& SetX(float x)  { x_ = x; return *this; }
    Layout& SetY(float y)  { y_ = y; return *this; }

    // ---------- 查询 ----------
    float X() const { return x_; }
    float Y() const { return y_; }
    float Gap_() const { return gap_; }
    float Cursor() const { return y_; }

    // ---------- 摆放一个组件到当前坐标，然后向下推进 ----------
    // 返回同一引用，支持链式：L.Row(a, 40).Row(b, 40);
    template<typename T>
    T& Row(T& w, float h) {
        w.SetPosition(x_, y_);
        y_ += h + gap_;
        return w;
    }

    // ---------- 只推进，不摆组件 ----------
    Layout& Skip(float h) {
        y_ += h + gap_;
        return *this;
    }

    // ---------- 换列 ----------
    Layout& NewColumn(float x, float y) {
        x_ = x; y_ = y;
        return *this;
    }

    // ---------- 只换 X，Y 保持 ----------
    Layout& SetXOnly(float x) {
        x_ = x;
        return *this;
    }

    // ---------- 拿当前位置给外部用 ----------
    void Current(float& outX, float& outY) const {
        outX = x_; outY = y_;
    }

    // ---------- 手动回退 Y（比如需要重排） ----------
    Layout& Rewind(float dy) {
        y_ -= dy;
        return *this;
    }

    // ---------- 手动前进 Y（不带 gap） ----------
    Layout& AdvanceRaw(float dy) {
        y_ += dy;
        return *this;
    }
};

} // namespace aur
