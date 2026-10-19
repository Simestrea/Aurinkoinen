#pragma once
#include "ui_root.hpp"
#include "raylib.h"
#include "init.hpp"
#include "text.hpp"
#include <string>
#include <cstddef>

namespace aur {

// ==========================================
// TextBox：可编辑的单行文本输入框
//
// 特点：
//   - 点击获得焦点，键盘输入实时编辑
//   - 支持 UTF-8（中文输入自动按多字节处理）
//   - 失焦时显示 placeholder
//   - 全局同时只有一个 TextBox 获得焦点
//   - 退格支持长按重复删除
//
// 用法（在角色脚本里）：
//   aur::TextBox input("输入你的名字…", 200, 300, 400, 44, 20);
//   while (true) {
//       input.Draw();
//       if (input.Submitted()) {
//           auto name = input.ConsumeText();
//           c.Say("你好，" + name + "!", 2.0f);
//       }
//       co_await aur::WaitNextFrame{};
//   }
//
// 说明：
//   - Draw() 内部同时处理输入和绘制，所以必须在
//     virtualScreen.Begin() / End() 之间调用
//   - 需要中文显示/输入时用 SetFont("fonts/xxx.ttc")
// ==========================================
class TextBox : public Widget {
private:
    // ---------- 内容 ----------
    std::string text_{};
    std::string placeholder_{};
    int maxChars_{64};   // UTF-8 码点上限（中文按 1 个算）

    // ---------- 布局 ----------
    float x_{0.0f};
    float y_{0.0f};
    float width_{200.0f};
    float height_{40.0f};
    float padding_{8.0f};
    int fontSize_{20};

    // ---------- 样式 ----------
    std::string fontName_{};   // 空 = 默认字体
    Color textColor_{BLACK};
    Color bgColor_{RAYWHITE};
    Color borderColor_{LIGHTGRAY};
    Color focusBorderColor_{DARKGRAY};
    Color placeholderColor_{GRAY};

    // ---------- 状态 ----------
    bool isHidden_{false};
    bool submitted_{false};
    float cursorTimer_{0.0f};
    bool cursorVisible_{true};
    float backspaceHold_{0.0f};
    float backspaceRepeat_{0.0f};

    // ---------- 焦点（全局唯一） ----------
    static TextBox*& FocusedSlot() {
        static TextBox* p = nullptr;
        return p;
    }

    // ---------- UTF-8 辅助 ----------
    static int CodepointCount(const std::string& s) {
        int n = 0;
        for (unsigned char c : s) {
            if ((c & 0xC0) != 0x80) ++n;   // 只数前导字节
        }
        return n;
    }

    static void AppendCodepoint(std::string& s, int cp) {
        if (cp < 0x80) {
            s += static_cast<char>(cp);
        } else if (cp < 0x800) {
            s += static_cast<char>(0xC0 | (cp >> 6));
            s += static_cast<char>(0x80 | (cp & 0x3F));
        } else if (cp < 0x10000) {
            s += static_cast<char>(0xE0 | (cp >> 12));
            s += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
            s += static_cast<char>(0x80 | (cp & 0x3F));
        } else {
            s += static_cast<char>(0xF0 | (cp >> 18));
            s += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
            s += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
            s += static_cast<char>(0x80 | (cp & 0x3F));
        }
    }

    static void PopLastChar(std::string& s) {
        if (s.empty()) return;
        size_t i = s.size();
        while (i > 0) {
            --i;
            if ((s[i] & 0xC0) != 0x80) { s.erase(i); return; }
        }
        s.clear();
    }

    Font ResolveFont() const {
        if (fontName_.empty()) return GetFontDefault();
        return detail::fontPool.GetOrLoad(fontName_);
    }

    bool TouchingMouse() const {
        const Vector2 m = detail::virtualScreen.GetVirtualMousePosition();
        return m.x >= x_ && m.x <= x_ + width_ &&
               m.y >= y_ && m.y <= y_ + height_;
    }

    void HandleKeyboard(float dt) {
        // 打字输入
        int cp;
        while ((cp = GetCharPressed()) > 0) {
            if (CodepointCount(text_) < maxChars_) {
                AppendCodepoint(text_, cp);
            }
        }

        // 退格（按下瞬间 + 长按重复）
        bool doBackspace = false;
        if (IsKeyPressed(KEY_BACKSPACE)) {
            doBackspace = true;
            backspaceHold_ = 0.0f;
            backspaceRepeat_ = 0.0f;
        } else if (IsKeyDown(KEY_BACKSPACE)) {
            backspaceHold_ += dt;
            if (backspaceHold_ > 0.4f) {
                backspaceRepeat_ += dt;
                if (backspaceRepeat_ > 0.05f) {
                    backspaceRepeat_ = 0.0f;
                    doBackspace = true;
                }
            }
        } else {
            backspaceHold_ = 0.0f;
            backspaceRepeat_ = 0.0f;
        }
        if (doBackspace) PopLastChar(text_);

        // 提交 / 取消
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) {
            submitted_ = true;
        }
        if (IsKeyPressed(KEY_ESCAPE)) {
            FocusedSlot() = nullptr;
        }
    }

public:
    TextBox(const std::string& placeholder,
            float x, float y, float width, float height,
            int fontSize = 20)
        : placeholder_{placeholder},
          x_{x}, y_{y}, width_{width}, height_{height},
          fontSize_{fontSize}
    {}

    ~TextBox() {
        if (FocusedSlot() == this) FocusedSlot() = nullptr;
    }

    TextBox(const TextBox&) = delete;
    TextBox& operator=(const TextBox&) = delete;
    TextBox(TextBox&&) = delete;
    TextBox& operator=(TextBox&&) = delete;

    // ---------- 内容 ----------
    void SetText(const std::string& s) { text_ = s; }
    const std::string& GetText() const noexcept { return text_; }
    void Clear() { text_.clear(); submitted_ = false; }
    bool IsEmpty() const noexcept { return text_.empty(); }
    int Length() const { return CodepointCount(text_); }

    // ---------- 布局 ----------
    void SetPosition(float x, float y) { x_ = x; y_ = y; }
    void SetSize(float w, float h) { width_ = w; height_ = h; }
    void SetFontSize(int s) { fontSize_ = s; }
    void SetMaxChars(int n) { maxChars_ = n; }

    // ---------- 样式 ----------
    void SetFont(const std::string& path) { fontName_ = path; }
    void SetPlaceholder(const std::string& p) { placeholder_ = p; }
    void SetTextColor(Color c) { textColor_ = c; }
    void SetBackgroundColor(Color c) { bgColor_ = c; }
    void SetBorderColor(Color c) { borderColor_ = c; }
    void SetFocusBorderColor(Color c) { focusBorderColor_ = c; }
    void SetPlaceholderColor(Color c) { placeholderColor_ = c; }

    // ---------- 焦点 ----------
    void Focus() { FocusedSlot() = this; }
    void Blur()  { if (FocusedSlot() == this) FocusedSlot() = nullptr; }
    bool IsFocused() const noexcept { return FocusedSlot() == this; }

    // ---------- 显示 ----------
    void Show() { isHidden_ = false; }
    void Hide() { isHidden_ = true; }
    bool IsHidden() const noexcept { return isHidden_; }

    // ---------- 提交 ----------
    // 按 Enter 后 Submitted() 为 true，直到 ClearSubmitted / ConsumeText 清掉
    bool Submitted() const noexcept { return submitted_; }
    void ClearSubmitted() { submitted_ = false; }

    // 取走内容并清空（适合"提交后就把框清空"的场景）
    std::string ConsumeText() {
        std::string r = text_;
        text_.clear();
        submitted_ = false;
        return r;
    }

    // ---------- Widget 接口 ----------
    Rectangle GetBounds() const override {
        return { x_, y_, width_, height_ };
    }

    bool IsFocusable() const override { return true; }

    void OnFocusGained() override { FocusedSlot() = this; }

    void OnFocusLost() override {
        if (FocusedSlot() == this) FocusedSlot() = nullptr;
    }

    // ---------- 绘制 + 输入 ----------
    void Draw() override {
        if (isHidden_) return;

        const float dt = GetFrameTime();

        // 1) 只有焦点框才处理键盘
        if (FocusedSlot() == this) {
            HandleKeyboard(dt);
        }

        // 2) 光标闪烁
        if (FocusedSlot() == this) {
            cursorTimer_ += dt;
            if (cursorTimer_ >= 0.5f) {
                cursorTimer_ -= 0.5f;
                cursorVisible_ = !cursorVisible_;
            }
        } else {
            cursorTimer_ = 0.0f;
            cursorVisible_ = true;
        }

        // 3) 绘制
        const bool focused = (FocusedSlot() == this);
        const Rectangle bounds{ x_, y_, width_, height_ };

        DrawRectangleRec(bounds, bgColor_);
        DrawRectangleLinesEx(bounds, 2.0f,
                             focused ? focusBorderColor_ : borderColor_);

        Font font = ResolveFont();
        const bool empty = text_.empty();
        const char* display = empty ? placeholder_.c_str() : text_.c_str();
        const Color color  = empty ? placeholderColor_ : textColor_;

        const float textY = y_ + (height_ - static_cast<float>(fontSize_)) / 2.0f;
        DrawTextEx(font, display,
                   { x_ + padding_, textY },
                   static_cast<float>(fontSize_), 1.0f, color);

        // 光标
        if (focused && cursorVisible_) {
            float cursorX = x_ + padding_;
            if (!empty) {
                Vector2 sz = MeasureTextEx(font, text_.c_str(),
                                           static_cast<float>(fontSize_), 1.0f);
                cursorX += sz.x;
            }
            const float top = textY;
            const float bot = textY + static_cast<float>(fontSize_);
            DrawLine(static_cast<int>(cursorX),
                     static_cast<int>(top),
                     static_cast<int>(cursorX),
                     static_cast<int>(bot),
                     textColor_);
        }
    }
};

} // namespace aur