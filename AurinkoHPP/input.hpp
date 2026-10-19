#pragma once
#include "raylib.h"
#include "init.hpp"

namespace aur::input {

// ==========================================
// 输入查询
//
// 鼠标坐标自动换算成虚拟画布坐标
// ==========================================

inline Vector2 MousePosition() {
    return detail::virtualScreen.GetVirtualMousePosition();
}

inline bool MousePressed(int button = MOUSE_BUTTON_LEFT) {
    return IsMouseButtonPressed(button);
}
inline bool MouseDown(int button = MOUSE_BUTTON_LEFT) {
    return IsMouseButtonDown(button);
}
inline bool MouseReleased(int button = MOUSE_BUTTON_LEFT) {
    return IsMouseButtonReleased(button);
}

inline bool KeyPressed(int key) { return IsKeyPressed(key); }
inline bool KeyDown(int key)    { return IsKeyDown(key); }
inline bool KeyReleased(int key){ return IsKeyReleased(key); }

} // namespace aur::input