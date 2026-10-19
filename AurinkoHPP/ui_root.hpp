#pragma once
#include "raylib.h"
#include "init.hpp"
#include <vector>
#include <algorithm>
#include <functional>

namespace aur {

// ==========================================
// Widget：UI 控件基类
//
// 任何想被 UIRoot 管理的控件，继承这个类并实现：
//   - GetBounds()  返回控件的矩形范围
//   - Draw()       绘制自己（内部用 IsHovered/IsFocused 判断状态）
//
// 可选覆盖：
//   - IsFocusable()       是否接受键盘焦点（默认 false）
//   - OnMouseEnter/Exit() 鼠标进出时的回调
//   - OnClick()           被点击时的回调（焦点转移后触发）
//   - OnFocusGained/Lost() 焦点变化时的回调
// ==========================================
class Widget {
public:
    virtual ~Widget() = default;

    virtual Rectangle GetBounds() const = 0;
    virtual void Draw() = 0;

    virtual bool IsFocusable() const { return false; }
    virtual void OnMouseEnter() {}
    virtual void OnMouseExit() {}
    virtual void OnClick() {}
    virtual void OnFocusGained() {}
    virtual void OnFocusLost() {}

    bool IsHovered() const noexcept { return hovered_; }
    bool IsFocused() const noexcept { return focused_; }

private:
    friend class UIRoot;
    bool hovered_ = false;
    bool focused_ = false;
};

// ==========================================
// UIRoot：控件容器 + 输入仲裁器
//
// 职责：
//   - 每帧找"当前被鼠标悬停的那个控件"（从上层往下找）
//   - 鼠标点击时，把点击事件分发给悬停的控件
//   - 管理键盘焦点：Tab 切换、点空白失焦、点控件转移焦点
//   - 按添加顺序绘制所有控件
//
// 用法：
//   aur::UIRoot ui;
//
//   // 循环外：注册控件
//   ui.Add(&startBtn);
//   ui.Add(&tbNp);
//   ui.Add(&tbW0);
//   // ...
//
//   // 循环内：
//   ui.Update();
//   ui.Draw();
//
// 注意：
//   - Add 的顺序 = 绘制顺序 = 命中优先级（后添加的在上层）
//   - widget 的生命周期由调用者管理（UIRoot 不拥有）
//   - 一次 Update 只认"最上面那一个"命中控件，避免重叠时多个同时响应
// ==========================================
class UIRoot {
private:
    std::vector<Widget*> widgets_;
    Widget* hovered_ = nullptr;
    Widget* focused_ = nullptr;

public:
    // ==========================================
    // 注册
    // ==========================================
    void Add(Widget* w) {
        if (w) widgets_.push_back(w);
    }

    void Remove(Widget* w) {
        auto it = std::find(widgets_.begin(), widgets_.end(), w);
        if (it != widgets_.end()) {
            if (hovered_ == w) hovered_ = nullptr;
            if (focused_ == w) focused_ = nullptr;
            widgets_.erase(it);
        }
    }

    void Clear() {
        widgets_.clear();
        hovered_ = nullptr;
        focused_ = nullptr;
    }

    // ==========================================
    // 每帧调用
    // ==========================================
    void Update() {
        const Vector2 mouse = detail::virtualScreen.GetVirtualMousePosition();

        // ---------- 1. 找 hovered（从上层往下） ----------
        Widget* newHovered = nullptr;
        for (auto it = widgets_.rbegin(); it != widgets_.rend(); ++it) {
            Widget* w = *it;
            if (!w) continue;
            if (CheckCollisionPointRec(mouse, w->GetBounds())) {
                newHovered = w;
                break;
            }
        }

        if (newHovered != hovered_) {
            if (hovered_) {
                hovered_->hovered_ = false;
                hovered_->OnMouseExit();
            }
            hovered_ = newHovered;
            if (hovered_) {
                hovered_->hovered_ = true;
                hovered_->OnMouseEnter();
            }
        }

        // ---------- 2. 点击 ----------
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (hovered_) {
                // 焦点转移
                if (hovered_->IsFocusable() && focused_ != hovered_) {
                    if (focused_) {
                        focused_->focused_ = false;
                        focused_->OnFocusLost();
                    }
                    focused_ = hovered_;
                    focused_->focused_ = true;
                    focused_->OnFocusGained();
                }
                // 点击回调
                hovered_->OnClick();
            } else {
                // 点击空白处失焦
                if (focused_) {
                    focused_->focused_ = false;
                    focused_->OnFocusLost();
                    focused_ = nullptr;
                }
            }
        }

        // ---------- 3. Tab 切换焦点 ----------
        if (IsKeyPressed(KEY_TAB)) {
            std::vector<Widget*> focusables;
            for (auto* w : widgets_) {
                if (w && w->IsFocusable()) focusables.push_back(w);
            }
            if (!focusables.empty()) {
                int cur = -1;
                for (size_t i = 0; i < focusables.size(); ++i) {
                    if (focusables[i] == focused_) { cur = (int)i; break; }
                }
                int next = (cur + 1) % (int)focusables.size();
                if (focused_) {
                    focused_->focused_ = false;
                    focused_->OnFocusLost();
                }
                focused_ = focusables[next];
                focused_->focused_ = true;
                focused_->OnFocusGained();
            }
        }
    }

    void Draw() {
        for (auto* w : widgets_) {
            if (w) w->Draw();
        }
    }

    // ==========================================
    // 查询
    // ==========================================
    Widget* Hovered() const noexcept { return hovered_; }
    Widget* Focused() const noexcept { return focused_; }

    // 用户主动设置焦点
    void SetFocus(Widget* w) {
        if (focused_ == w) return;
        if (focused_) {
            focused_->focused_ = false;
            focused_->OnFocusLost();
        }
        focused_ = w;
        if (focused_) {
            focused_->focused_ = true;
            focused_->OnFocusGained();
        }
    }

    void ClearFocus() { SetFocus(nullptr); }
};

} // namespace aur