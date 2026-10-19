#pragma once
#include "costume.hpp"

namespace aur {

// ==========================================
// CharacterWithTransform: position / direction / scale
// ==========================================
class CharacterWithTransform : public CharacterWithCostume {
public:
    using CharacterWithCostume::CharacterWithCostume;

    Vector2 GetPosition() const noexcept { return position_; }
    float   GetDirection() const noexcept { return direction_; }
    float   GetScale() const noexcept { return scale_; }
    bool    IsHidden() const noexcept { return isHidden_; }
    int     GetZIndex() const noexcept { return zIndex_; }

    int   GetFrameCount() const noexcept { return frameCount_; }
    int   GetCurrentFrame() const noexcept { return currentFrame_; }
    float GetFrameSpeed() const noexcept { return frameSpeed_; }
    float GetFrameWidth() const noexcept { return frameWidth_; }
    float GetFrameHeight() const noexcept { return frameHeight_; }
    float GetDisplayWidth() const noexcept { return displayWidth_; }
    float GetDisplayHeight() const noexcept { return displayHeight_; }

    bool IsGliding() const noexcept { return isGliding_; }
    bool IsDragging() const noexcept { return isDragging_; }
    bool IsThinking() const noexcept { return isThinking_; }

    Color GetColorEffect() const noexcept { return colorEffect_; }
    const std::string& GetBubbleText() const noexcept { return bubbleText_; }
    float GetBubbleTimer() const noexcept { return bubbleTimer_; }

    float GetVolume() const noexcept { return volume_; }
    float GetPitch() const noexcept { return pitch_; }
    const std::string& GetAnswer() const noexcept { return answer_; }

    void SetPosition(Vector2 p) { position_ = p; isGliding_ = false; }
    void SetX(float x) { position_.x = x; isGliding_ = false; }
    void SetY(float y) { position_.y = y; isGliding_ = false; }
    void SetDirection(float d) { direction_ = d; }
    void SetZIndex(int z) { zIndex_ = z; }

    void SetScale(float s) {
        scale_ = s;
        displayWidth_ = frameWidth_ * scale_;
        displayHeight_ = frameHeight_ * scale_;
    }
};

} // namespace aur
