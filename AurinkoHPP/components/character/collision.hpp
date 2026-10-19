#pragma once
#include "sound.hpp"
#include <aurinko/init.hpp>

namespace aur {

// ==========================================
// CharacterWithCollision: collision + input detection
// ==========================================
class CharacterWithCollision : public CharacterWithSound {
public:
    using CharacterWithSound::CharacterWithSound;

    Rectangle GetBoundingBox() const noexcept {
        return {
            position_.x - displayWidth_ / 2.0f,
            position_.y - displayHeight_ / 2.0f,
            displayWidth_,
            displayHeight_
        };
    }

    float GetCollisionRadius() const noexcept {
        const float shorter = (displayWidth_ < displayHeight_)
                              ? displayWidth_ : displayHeight_;
        return shorter / 2.0f;
    }

    bool Touching(const CharacterWithCollision& other) const noexcept {
        const float dx = position_.x - other.position_.x;
        const float dy = position_.y - other.position_.y;
        const float distSq = dx * dx + dy * dy;
        const float radiusSum = GetCollisionRadius() + other.GetCollisionRadius();
        return distSq < radiusSum * radiusSum;
    }

    bool TouchingBox(const CharacterWithCollision& other) const noexcept {
        return CheckCollisionRecs(GetBoundingBox(), other.GetBoundingBox());
    }

    bool TouchingMouse() const {
        const Vector2 mouse = detail::virtualScreen.GetVirtualMousePosition();
        return mouse.x > position_.x - displayWidth_ / 2.0f &&
               mouse.x < position_.x + displayWidth_ / 2.0f &&
               mouse.y > position_.y - displayHeight_ / 2.0f &&
               mouse.y < position_.y + displayHeight_ / 2.0f;
    }

    float GetDistanceToMouse() const {
        return Vector2Distance(position_,
                               detail::virtualScreen.GetVirtualMousePosition());
    }

    void SetDragMode(bool canDrag) { draggable_ = canDrag; }

    void AskAndWait(const std::string& question) {
        TraceLog(LOG_INFO, TextFormat("Ask: %s", question.c_str()));
        answer_ = "UserInput";
    }
};

} // namespace aur
