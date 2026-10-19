#pragma once
#include "motion.hpp"
#include <aurinko/coroutine.hpp>
#include <aurinko/easing.hpp>

namespace aur {

// ==========================================
// CharacterWithAppearance: appearance
// ==========================================
class CharacterWithAppearance : public CharacterWithMotion {
public:
    using CharacterWithMotion::CharacterWithMotion;

    void Say(const std::string& text, float duration = 2.0f) {
        bubbleText_ = text;
        bubbleTimer_ = duration;
        isThinking_ = false;
    }
    void Think(const std::string& text, float duration = 2.0f) {
        bubbleText_ = text;
        bubbleTimer_ = duration;
        isThinking_ = true;
    }

    void ChangeSize(float amount) { SetScale(scale_ + amount / 100.0f); }
    void SetSize(float size)      { SetScale(size / 100.0f); }

    void ChangeColorEffect(float amount) {
        colorEffect_.a = static_cast<unsigned char>(
            fmaxf(0.0f, fminf(255.0f, colorEffect_.a + amount)));
    }
    void SetColorEffect(float value) {
        colorEffect_.a = static_cast<unsigned char>(
            fmaxf(0.0f, fminf(255.0f, value)));
    }
    void ClearEffects() { colorEffect_ = WHITE; }

    void Show()         { isHidden_ = false; }
    void Hide()         { isHidden_ = true; }
    void ToggleHidden() { isHidden_ = !isHidden_; }

    void GoToFront()                 { zIndex_ = 999; }
    void GoForwardLayers(int layers) { zIndex_ += layers; }

    // ==========================================
    // 动画（等待完成的协程动作）
    // ==========================================
    // 用法：
    //   co_await c.FadeIn(0.5f);
    //   co_await c.MoveTo(300, 200, 0.8f, aur::ease::OutQuad);
    //   co_await aur::Parallel({
    //       c.FadeIn(0.3f),
    //       c.ScaleTo(1.5f, 0.3f),
    //   });

    // 淡入：透明度 0 → 255
    Tween FadeIn(float duration = 0.3f, float (*ease)(float) = nullptr) {
        return Tween{ duration, [this](float t) {
            SetColorEffect(255.0f * t);
        }, ease };
    }

    // 淡出：透明度 → 0
    Tween FadeOut(float duration = 0.3f, float (*ease)(float) = nullptr) {
        return Tween{ duration, [this](float t) {
            SetColorEffect(255.0f * (1.0f - t));
        }, ease };
    }

    // 缩放到指定倍数
    Tween ScaleTo(float target, float duration = 0.3f,
                  float (*ease)(float) = nullptr) {
        float start = scale_;
        return Tween{ duration, [this, start, target](float t) {
            scale_ = start + (target - start) * t;
            displayWidth_  = frameWidth_  * scale_;
            displayHeight_ = frameHeight_ * scale_;
        }, ease };
    }

    // 移动到指定坐标
    Tween MoveTo(float x, float y, float duration = 0.5f,
                 float (*ease)(float) = nullptr) {
        Vector2 start  = position_;
        Vector2 target = { x, y };
        return Tween{ duration, [this, start, target](float t) {
            position_ = Vector2Lerp(start, target, t);
            isGliding_ = false;
        }, ease };
    }
};

} // namespace aur
