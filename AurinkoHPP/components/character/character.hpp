#pragma once
#include "collision.hpp"

namespace aur {

// ==========================================
// Character: final composed class
//   Core -> Costume -> Transform -> Motion
//        -> Appearance -> Sound -> Collision
// ==========================================
class Character : public CharacterWithCollision {
public:
    // ── 带纹理（原有）──
    Character(const std::string& name,
              const char* texturePath,
              Vector2 startPos,
              float startScale,
              int startFrameCount,
              float startFrameSpeed = 0.15f)
        : CharacterWithCollision(name, startPos, startScale)
    {
        AddCostume(texturePath, startFrameCount, startFrameSpeed);
    }

    // ── 无纹理（新增）──
    // 纯逻辑角色：只作为协程载体 / UI 控制器，不绘制精灵图。
    // 由 codegen 在 config.txt 的 texture 字段为 "-" 时自动选用。
    Character(const std::string& name, Vector2 startPos = {0.0f, 0.0f})
        : CharacterWithCollision(name, startPos, 1.0f)
    {
        // 不调用 AddCostume
    }

    void Update(float deltaTime, int screenWidth, int screenHeight) {
        if (isHidden_) return;
        if (!isActive_) return;

        if (isGliding_) {
            glideTimer_ += deltaTime;
            float t = glideTimer_ / glideDuration_;
            if (t >= 1.0f) { t = 1.0f; isGliding_ = false; }
            position_ = Vector2Lerp(glideStart_, glideTarget_, t);
        }

        frameTimer_ += deltaTime;
        if (frameTimer_ >= frameSpeed_) {
            frameTimer_ = 0.0f;
            currentFrame_ = (currentFrame_ + 1) % frameCount_;
        }

        if (bubbleTimer_ > 0.0f) {
            bubbleTimer_ -= deltaTime;
            if (bubbleTimer_ <= 0.0f) bubbleText_.clear();
        }

        if (draggable_) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && TouchingMouse())
                isDragging_ = true;
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
                isDragging_ = false;
            if (isDragging_) {
                position_ = detail::virtualScreen.GetVirtualMousePosition();
                isGliding_ = false;
            }
        }

        if (position_.x - displayWidth_ / 2.0f < 0.0f)
            position_.x = displayWidth_ / 2.0f;
        if (position_.x + displayWidth_ / 2.0f > static_cast<float>(screenWidth))
            position_.x = static_cast<float>(screenWidth) - displayWidth_ / 2.0f;
        if (position_.y - displayHeight_ / 2.0f < 0.0f)
            position_.y = displayHeight_ / 2.0f;
        if (position_.y + displayHeight_ / 2.0f > static_cast<float>(screenHeight))
            position_.y = static_cast<float>(screenHeight) - displayHeight_ / 2.0f;
    }

    void Draw() const {
        if (isHidden_) return;
        if (!isActive_) return;
        if (costumes_.empty()) return;      // 无纹理角色直接跳过

        const Costume& cos = costumes_[currentCostume_];

        Rectangle sourceRec{
            static_cast<float>(currentFrame_) * cos.frameWidth,
            0.0f,
            cos.frameWidth,
            cos.frameHeight
        };

        float rotationAngle = 0.0f;

        if (rotateWithDirection_) {
            rotationAngle = direction_;
        } else {
            if (flipHorizontal_) {
                float normalizedDir = fmodf(direction_, 360.0f);
                if (normalizedDir < 0.0f) normalizedDir += 360.0f;
                if (normalizedDir > 90.0f && normalizedDir < 270.0f) {
                    sourceRec.width = -sourceRec.width;
                }
            }
        }

        const Rectangle destRec{ position_.x, position_.y,
                                 displayWidth_, displayHeight_ };
        const Vector2 origin{ displayWidth_ / 2.0f, displayHeight_ / 2.0f };

        DrawTexturePro(*cos.texture, sourceRec, destRec, origin,
                       rotationAngle, colorEffect_);
    }

    void DrawBubble() const {
        if (isHidden_) return;
        if (!isActive_) return;
        if (bubbleText_.empty()) return;

        const Vector2 textSize = MeasureTextEx(GetFontDefault(),
                                                bubbleText_.c_str(), 20, 1);
        const Vector2 bubblePos{
            position_.x - textSize.x / 2.0f,
            position_.y - displayHeight_ / 2.0f - 30.0f
        };

        DrawRectangleRounded(
            { bubblePos.x - 10, bubblePos.y - 5,
              textSize.x + 20, textSize.y + 10 },
            0.5f, 10, LIGHTGRAY);

        if (isThinking_) {
            DrawCircle(
                static_cast<int>(bubblePos.x + textSize.x / 2.0f),
                static_cast<int>(bubblePos.y + textSize.y + 15.0f),
                5, LIGHTGRAY);
        }

        DrawText(bubbleText_.c_str(),
                 static_cast<int>(bubblePos.x),
                 static_cast<int>(bubblePos.y),
                 20, DARKGRAY);
    }
};

} // namespace aur