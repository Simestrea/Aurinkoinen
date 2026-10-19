#pragma once
#include "transform.hpp"
#include <aurinko/init.hpp>

namespace aur {

// ==========================================
// CharacterWithMotion: motion
// ==========================================
class CharacterWithMotion : public CharacterWithTransform {
public:
    using CharacterWithTransform::CharacterWithTransform;

    void MoveSteps(float steps) {
        const float radians = direction_ * DEG2RAD;
        position_.x += cosf(radians) * steps;
        position_.y += sinf(radians) * steps;
    }

    void TurnRight(float degrees) { direction_ += degrees; }
    void TurnLeft(float degrees)  { direction_ -= degrees; }

    void ChangeX(float dx) { position_.x += dx; isGliding_ = false; }
    void ChangeY(float dy) { position_.y += dy; isGliding_ = false; }

    void GoToXY(float x, float y) {
        position_ = { x, y };
        isGliding_ = false;
    }

    void GoToRandomPosition(int screenWidth, int screenHeight) {
        const float minX = displayWidth_ / 2.0f;
        const float maxX = static_cast<float>(screenWidth) - displayWidth_ / 2.0f;
        const float minY = displayHeight_ / 2.0f;
        const float maxY = static_cast<float>(screenHeight) - displayHeight_ / 2.0f;
        position_.x = static_cast<float>(GetRandomValue(static_cast<int>(minX), static_cast<int>(maxX)));
        position_.y = static_cast<float>(GetRandomValue(static_cast<int>(minY), static_cast<int>(maxY)));
        isGliding_ = false;
    }

    void GlideToXY(float seconds, float x, float y) {
        isGliding_ = true;
        glideStart_ = position_;
        glideTarget_ = { x, y };
        glideDuration_ = seconds;
        glideTimer_ = 0.0f;
    }

    void GlideToRandomPosition(float seconds, int screenWidth, int screenHeight) {
        const float minX = displayWidth_ / 2.0f;
        const float maxX = static_cast<float>(screenWidth) - displayWidth_ / 2.0f;
        const float minY = displayHeight_ / 2.0f;
        const float maxY = static_cast<float>(screenHeight) - displayHeight_ / 2.0f;
        const float targetX = static_cast<float>(GetRandomValue(static_cast<int>(minX), static_cast<int>(maxX)));
        const float targetY = static_cast<float>(GetRandomValue(static_cast<int>(minY), static_cast<int>(maxY)));
        GlideToXY(seconds, targetX, targetY);
    }

    void PointInDirection(float newDirection) { direction_ = newDirection; }

    void PointTowardsMouse() {
        const Vector2 mousePos = detail::virtualScreen.GetVirtualMousePosition();
        direction_ = atan2f(mousePos.y - position_.y,
                            mousePos.x - position_.x) * RAD2DEG;
    }

    void BounceOffEdge(int screenWidth, int screenHeight) {
        if (position_.x - displayWidth_ / 2.0f < 0.0f) {
            position_.x = displayWidth_ / 2.0f;
            direction_ = 180.0f - direction_;
        }
        if (position_.x + displayWidth_ / 2.0f > static_cast<float>(screenWidth)) {
            position_.x = static_cast<float>(screenWidth) - displayWidth_ / 2.0f;
            direction_ = 180.0f - direction_;
        }
        if (position_.y - displayHeight_ / 2.0f < 0.0f) {
            position_.y = displayHeight_ / 2.0f;
            direction_ = -direction_;
        }
        if (position_.y + displayHeight_ / 2.0f > static_cast<float>(screenHeight)) {
            position_.y = static_cast<float>(screenHeight) - displayHeight_ / 2.0f;
            direction_ = -direction_;
        }
    }
};

} // namespace aur
