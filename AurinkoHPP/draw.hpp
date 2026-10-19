#pragma once
#include "raylib.h"
#include <algorithm>

namespace aur::draw {

// ==========================================
// 绘制原语
//
// 目的：让角色代码不直接调 raylib 函数。
//   - 未来换渲染后端只需改这一处
//   - 提供 float / Vector2 / Rectangle 多种重载
//
// 类型（Color / Vector2 / Rectangle）仍用 raylib 的，
// 它们是基础数据结构，替换收益低。
// ==========================================

// ---------- 颜色 ----------
inline Color Fade(Color c, float alpha) {
    c.a = (unsigned char)(std::clamp(alpha, 0.0f, 1.0f) * 255.0f);
    return c;
}

inline Color WithAlpha(Color c, unsigned char a) {
    c.a = a;
    return c;
}

// ---------- 矩形 ----------
inline void Rect(float x, float y, float w, float h, Color c) {
    DrawRectangle((int)x, (int)y, (int)w, (int)h, c);
}
inline void Rect(Rectangle r, Color c) {
    DrawRectangleRec(r, c);
}
inline void RectOutline(float x, float y, float w, float h,
                        Color c, float thickness = 1.0f) {
    DrawRectangleLinesEx({ x, y, w, h }, thickness, c);
}
inline void RectOutline(Rectangle r, Color c, float thickness = 1.0f) {
    DrawRectangleLinesEx(r, thickness, c);
}

// ---------- 圆 ----------
inline void Circle(float cx, float cy, float radius, Color c) {
    DrawCircle((int)cx, (int)cy, radius, c);
}
inline void Circle(Vector2 center, float radius, Color c) {
    DrawCircleV(center, radius, c);
}
inline void CircleOutline(float cx, float cy, float radius, Color c) {
    DrawCircleLines((int)cx, (int)cy, radius, c);
}
inline void CircleOutline(Vector2 center, float radius, Color c) {
    DrawCircleLinesV(center, radius, c);
}

// ---------- 线 ----------
inline void Line(float x1, float y1, float x2, float y2, Color c) {
    DrawLine((int)x1, (int)y1, (int)x2, (int)y2, c);
}
inline void Line(Vector2 a, Vector2 b, Color c) {
    DrawLineV(a, b, c);
}
inline void LineThick(Vector2 a, Vector2 b, float thickness, Color c) {
    DrawLineEx(a, b, thickness, c);
}
inline void LineThick(float x1, float y1, float x2, float y2,
                      float thickness, Color c) {
    DrawLineEx({ x1, y1 }, { x2, y2 }, thickness, c);
}

// ---------- 命中检测 ----------
inline bool PointInRect(Vector2 p, Rectangle r) {
    return CheckCollisionPointRec(p, r);
}
inline bool PointInRect(float px, float py, Rectangle r) {
    return CheckCollisionPointRec({ px, py }, r);
}
inline bool PointInCircle(Vector2 p, Vector2 center, float radius) {
    float dx = p.x - center.x;
    float dy = p.y - center.y;
    return dx * dx + dy * dy <= radius * radius;
}

} // namespace aur::draw