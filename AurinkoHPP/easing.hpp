#pragma once
// ==========================================
// aur::ease —— 缓动函数
//
// 用法：
//   co_await c.MoveTo(300, 200, 0.8f, aur::ease::OutQuad);
//   co_await c.FadeIn(0.5f, aur::ease::OutCubic);
//
// 所有函数签名：float(float t)，t ∈ [0, 1]，返回缓动后的值
// ==========================================
#include <cmath>

namespace aur::ease {

using EaseFn = float(*)(float);

// ---------- 线性 ----------
inline float Linear(float t) { return t; }

// ---------- Quad ----------
inline float InQuad(float t)  { return t * t; }
inline float OutQuad(float t) { return 1.0f - (1.0f - t) * (1.0f - t); }
inline float InOutQuad(float t) {
    return t < 0.5f ? 2.0f * t * t
                    : 1.0f - 2.0f * (1.0f - t) * (1.0f - t);
}

// ---------- Cubic ----------
inline float InCubic(float t)  { return t * t * t; }
inline float OutCubic(float t) {
    float f = 1.0f - t;
    return 1.0f - f * f * f;
}
inline float InOutCubic(float t) {
    return t < 0.5f
        ? 4.0f * t * t * t
        : 1.0f - 4.0f * (1.0f - t) * (1.0f - t) * (1.0f - t);
}

// ---------- Quart ----------
inline float InQuart(float t)  { return t * t * t * t; }
inline float OutQuart(float t) {
    float f = 1.0f - t;
    return 1.0f - f * f * f * f;
}

// ---------- Quint ----------
inline float InQuint(float t)  { return t * t * t * t * t; }
inline float OutQuint(float t) {
    float f = 1.0f - t;
    return 1.0f - f * f * f * f * f;
}

// ---------- Sine ----------
inline float InSine(float t) {
    return 1.0f - std::cos(t * 3.14159265358979f * 0.5f);
}
inline float OutSine(float t) {
    return std::sin(t * 3.14159265358979f * 0.5f);
}
inline float InOutSine(float t) {
    return -(std::cos(3.14159265358979f * t) - 1.0f) * 0.5f;
}

// ---------- Back（回弹） ----------
inline float OutBack(float t) {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    float f = t - 1.0f;
    return 1.0f + c3 * f * f * f + c1 * f * f;
}
inline float InBack(float t) {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    return c3 * t * t * t - c1 * t * t;
}

// ---------- Elastic（弹性） ----------
inline float OutElastic(float t) {
    if (t <= 0.0f) return 0.0f;
    if (t >= 1.0f) return 1.0f;
    const float c4 = 2.0f * 3.14159265358979f / 3.0f;
    return std::pow(2.0f, -10.0f * t)
         * std::sin((t * 10.0f - 0.75f) * c4) + 1.0f;
}

// ---------- Bounce（弹跳） ----------
inline float OutBounce(float t) {
    const float n1 = 7.5625f;
    const float d1 = 2.75f;
    if (t < 1.0f / d1) { return n1 * t * t; }
    if (t < 2.0f / d1) { t -= 1.5f / d1; return n1 * t * t + 0.75f; }
    if (t < 2.5f / d1) { t -= 2.25f / d1; return n1 * t * t + 0.9375f; }
    t -= 2.625f / d1;
    return n1 * t * t + 0.984375f;
}

} // namespace aur::ease
