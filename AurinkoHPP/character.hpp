#pragma once
#include "raylib.h"
#include "raymath.h"
#include "init.hpp"
#include <cmath>
#include <string>
#include <vector>
#include <utility>
#include <memory>
#include <unordered_map>

namespace aur {

class TexturePool {
private:
    std::unordered_map<std::string, std::shared_ptr<Texture2D>> pool_;

public:
    std::shared_ptr<Texture2D> Load(const std::string& path) {
        auto it = pool_.find(path);
        if (it != pool_.end()) return it->second;

        auto deleter = [](Texture2D* t) {
            if (t) { UnloadTexture(*t); delete t; }
        };
        auto tex = std::shared_ptr<Texture2D>(
            new Texture2D(LoadTexture(path.c_str())),
            deleter
        );
        pool_[path] = tex;
        return tex;
    }
};

// ==========================================
// 内部单例
// ==========================================
namespace detail {
    inline TexturePool texturePool;
}

class Character {
private:
    struct Costume {
        std::shared_ptr<Texture2D> texture{};
        std::string name{};
        int frameCount{10};
        float frameSpeed{0.15f};
        float frameWidth{0.0f};
        float frameHeight{0.0f};
    };

    std::vector<Costume> costumes_;
    int currentCostume_{0};

    Vector2 position_{};
    float direction_{0.0f};
    float scale_{1.0f};
    bool isHidden_{false};
    int zIndex_{0};

    std::string name_{};
    int sceneId_{0};
    bool isActive_{true};

    int frameCount_{10};
    int currentFrame_{0};
    float frameTimer_{0.0f};
    float frameSpeed_{0.15f};
    float frameWidth_{0.0f};
    float frameHeight_{0.0f};
    float displayWidth_{0.0f};
    float displayHeight_{0.0f};

    bool isGliding_{false};
    Vector2 glideStart_{};
    Vector2 glideTarget_{};
    float glideDuration_{0.0f};
    float glideTimer_{0.0f};

    bool flipHorizontal_{true};
    bool rotateWithDirection_{false};
    Color colorEffect_{WHITE};
    bool draggable_{false};
    bool isDragging_{false};

    std::string bubbleText_{};
    float bubbleTimer_{0.0f};
    bool isThinking_{false};

    float volume_{1.0f};
    float pitch_{1.0f};
    std::string answer_{};

    bool markedForDeletion_{false};
    bool alwaysOnTop_{false};

    void SyncCostumeToRuntime() {
        const Costume& cos = costumes_[currentCostume_];
        frameCount_ = cos.frameCount;
        frameSpeed_ = cos.frameSpeed;
        frameWidth_ = cos.frameWidth;
        frameHeight_ = cos.frameHeight;
        displayWidth_ = frameWidth_ * scale_;
        displayHeight_ = frameHeight_ * scale_;
    }

    void MoveFrom(Character&& other) noexcept {
        costumes_ = std::move(other.costumes_);
        other.costumes_.clear();

        currentCostume_ = other.currentCostume_;
        position_ = other.position_;
        direction_ = other.direction_;
        scale_ = other.scale_;
        isHidden_ = other.isHidden_;
        zIndex_ = other.zIndex_;

        name_ = std::move(other.name_);
        sceneId_ = other.sceneId_;
        isActive_ = other.isActive_;

        frameCount_ = other.frameCount_;
        currentFrame_ = other.currentFrame_;
        frameTimer_ = other.frameTimer_;
        frameSpeed_ = other.frameSpeed_;
        frameWidth_ = other.frameWidth_;
        frameHeight_ = other.frameHeight_;
        displayWidth_ = other.displayWidth_;
        displayHeight_ = other.displayHeight_;

        isGliding_ = other.isGliding_;
        glideStart_ = other.glideStart_;
        glideTarget_ = other.glideTarget_;
        glideDuration_ = other.glideDuration_;
        glideTimer_ = other.glideTimer_;

        flipHorizontal_ = other.flipHorizontal_;
        rotateWithDirection_ = other.rotateWithDirection_;
        colorEffect_ = other.colorEffect_;
        draggable_ = other.draggable_;
        isDragging_ = other.isDragging_;

        bubbleText_ = std::move(other.bubbleText_);
        bubbleTimer_ = other.bubbleTimer_;
        isThinking_ = other.isThinking_;

        volume_ = other.volume_;
        pitch_ = other.pitch_;
        answer_ = std::move(other.answer_);

        markedForDeletion_ = other.markedForDeletion_;
        alwaysOnTop_ = other.alwaysOnTop_;
    }

public:
    Character(const std::string& name,
              const char* texturePath,
              Vector2 startPos,
              float startScale,
              int startFrameCount,
              float startFrameSpeed = 0.15f)
        : name_{name}, position_{startPos}, scale_{startScale}
    {
        AddCostume(texturePath, startFrameCount, startFrameSpeed);
    }

    ~Character() = default;

    Character(const Character&) = default;
    Character& operator=(const Character&) = default;

    Character(Character&& other) noexcept { MoveFrom(std::move(other)); }

    Character& operator=(Character&& other) noexcept {
        if (this != &other) {
            costumes_.clear();
            MoveFrom(std::move(other));
        }
        return *this;
    }

    void Destroy() { markedForDeletion_ = true; }
    void SetAlwaysOnTop(bool on) { alwaysOnTop_ = on; }
    bool IsAlwaysOnTop() const noexcept { return alwaysOnTop_; }
    bool IsMarkedForDeletion() const noexcept { return markedForDeletion_; }

    // ==========================================
    // 名字与场景
    // ==========================================
    const std::string& GetName() const noexcept { return name_; }

    void SetScene(int id) noexcept { sceneId_ = id; }
    int GetScene() const noexcept { return sceneId_; }

    void SetActive(bool a) noexcept { isActive_ = a; }
    bool IsActive() const noexcept { return isActive_; }

    // ==========================================
    // 旋转模式
    // ==========================================
    // false（默认）：左右翻转模式。精灵图不会被旋转，只在朝向左侧时水平镜像。
    // true：真旋转模式。精灵图会跟随 direction_ 旋转。
    void SetRotateWithDirection(bool on) { rotateWithDirection_ = on; }
    bool IsRotateWithDirection() const noexcept { return rotateWithDirection_; }

    void SetFlipHorizontal(bool on) { flipHorizontal_ = on; }
    bool IsFlipHorizontal() const noexcept { return flipHorizontal_; }

    // ==========================================
    // 造型
    // ==========================================
    void AddCostume(const char* texturePath, int frameCount, float frameSpeed = 0.15f) {
        Costume cos;
        cos.texture = detail::texturePool.Load(texturePath);
        cos.name = texturePath;
        cos.frameCount = frameCount;
        cos.frameSpeed = frameSpeed;
        cos.frameWidth = static_cast<float>(cos.texture->width) / frameCount;
        cos.frameHeight = static_cast<float>(cos.texture->height);
        costumes_.push_back(cos);

        if (costumes_.size() == 1) {
            currentCostume_ = 0;
            SyncCostumeToRuntime();
        }
    }

    void SwitchCostume(int index) {
        if (index < 0 || index >= static_cast<int>(costumes_.size())) return;
        if (index == currentCostume_) return;
        currentCostume_ = index;
        currentFrame_ = 0;
        frameTimer_ = 0.0f;
        SyncCostumeToRuntime();
    }

    void NextCostume() {
        if (costumes_.empty()) return;
        SwitchCostume((currentCostume_ + 1) % static_cast<int>(costumes_.size()));
    }

    int GetCostumeCount() const noexcept {
        return static_cast<int>(costumes_.size());
    }

    int GetCurrentCostume() const noexcept { return currentCostume_; }

    float GetCostumeDuration() const noexcept {
        if (costumes_.empty()) return 0.0f;
        const Costume& cos = costumes_[currentCostume_];
        return cos.frameCount * cos.frameSpeed;
    }

    // ==========================================
    // 只读访问器
    // ==========================================
    Vector2 GetPosition() const noexcept { return position_; }
    float GetDirection() const noexcept { return direction_; }
    float GetScale() const noexcept { return scale_; }
    bool IsHidden() const noexcept { return isHidden_; }
    int GetZIndex() const noexcept { return zIndex_; }

    int GetFrameCount() const noexcept { return frameCount_; }
    int GetCurrentFrame() const noexcept { return currentFrame_; }
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

    // ==========================================
    // 写访问器
    // ==========================================
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

    // ==========================================
    // 运动
    // ==========================================
    void MoveSteps(float steps) {
        const float radians = direction_ * DEG2RAD;
        position_.x += cosf(radians) * steps;
        position_.y += sinf(radians) * steps;
    }

    void TurnRight(float degrees) { direction_ += degrees; }
    void TurnLeft(float degrees) { direction_ -= degrees; }

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
        direction_ = atan2f(mousePos.y - position_.y, mousePos.x - position_.x) * RAD2DEG;
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

    // ==========================================
    // 外观
    // ==========================================
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
    void SetSize(float size) { SetScale(size / 100.0f); }

    void ChangeColorEffect(float amount) {
        colorEffect_.a = static_cast<unsigned char>(fmaxf(0.0f, fminf(255.0f, colorEffect_.a + amount)));
    }

    void SetColorEffect(float value) {
        colorEffect_.a = static_cast<unsigned char>(fmaxf(0.0f, fminf(255.0f, value)));
    }

    void ClearEffects() { colorEffect_ = WHITE; }
    void Show() { isHidden_ = false; }
    void Hide() { isHidden_ = true; }
    void ToggleHidden() { isHidden_ = !isHidden_; }
    void GoToFront() { zIndex_ = 999; }
    void GoForwardLayers(int layers) { zIndex_ += layers; }

    // ==========================================
    // 音效
    // ==========================================
    void PlaySoundNormal(Sound sound) { PlaySound(sound); }
    void StopSoundEffect(Sound sound) { StopSound(sound); }

    void ChangePitch(Sound sound, float amount) {
        pitch_ += amount / 100.0f;
        SetSoundPitch(sound, pitch_);
    }
    void SetPitchValue(Sound sound, float value) {
        pitch_ = value / 100.0f;
        SetSoundPitch(sound, pitch_);
    }
    void ClearSoundEffects(Sound sound) {
        pitch_ = 1.0f;
        SetSoundPitch(sound, pitch_);
    }
    void ChangeVolume(Sound sound, float amount) {
        volume_ += amount / 100.0f;
        SetSoundVolume(sound, volume_);
    }
    void SetVolumeValue(Sound sound, float value) {
        volume_ = value / 100.0f;
        SetSoundVolume(sound, volume_);
    }

    // ==========================================
    // 碰撞检测
    // ==========================================
    Rectangle GetBoundingBox() const noexcept {
        return {
            position_.x - displayWidth_ / 2.0f,
            position_.y - displayHeight_ / 2.0f,
            displayWidth_,
            displayHeight_
        };
    }

    float GetCollisionRadius() const noexcept {
        const float shorter = (displayWidth_ < displayHeight_) ? displayWidth_ : displayHeight_;
        return shorter / 2.0f;
    }

    bool Touching(const Character& other) const noexcept {
        const float dx = position_.x - other.position_.x;
        const float dy = position_.y - other.position_.y;
        const float distSq = dx * dx + dy * dy;
        const float radiusSum = GetCollisionRadius() + other.GetCollisionRadius();
        return distSq < radiusSum * radiusSum;
    }

    bool TouchingBox(const Character& other) const noexcept {
        return CheckCollisionRecs(GetBoundingBox(), other.GetBoundingBox());
    }

    bool TouchingMouse() const {
        const Vector2 mouse = detail::virtualScreen.GetVirtualMousePosition();
        return mouse.x > position_.x - displayWidth_ / 2.0f &&
               mouse.x < position_.x + displayWidth_ / 2.0f &&
               mouse.y > position_.y - displayHeight_ / 2.0f &&
               mouse.y < position_.y + displayHeight_ / 2.0f;
    }

    // ==========================================
    // 输入侦测
    // ==========================================
    float GetDistanceToMouse() const {
        return Vector2Distance(position_, detail::virtualScreen.GetVirtualMousePosition());
    }

    void SetDragMode(bool canDrag) { draggable_ = canDrag; }

    void AskAndWait(const std::string& question) {
        TraceLog(LOG_INFO, TextFormat("Ask: %s", question.c_str()));
        answer_ = "UserInput";
    }

    // ==========================================
    // 核心循环
    // ==========================================
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
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && TouchingMouse()) isDragging_ = true;
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) isDragging_ = false;
            if (isDragging_) {
                position_ = detail::virtualScreen.GetVirtualMousePosition();
                isGliding_ = false;
            }
        }

        if (position_.x - displayWidth_ / 2.0f < 0.0f) position_.x = displayWidth_ / 2.0f;
        if (position_.x + displayWidth_ / 2.0f > static_cast<float>(screenWidth))
            position_.x = static_cast<float>(screenWidth) - displayWidth_ / 2.0f;
        if (position_.y - displayHeight_ / 2.0f < 0.0f) position_.y = displayHeight_ / 2.0f;
        if (position_.y + displayHeight_ / 2.0f > static_cast<float>(screenHeight))
            position_.y = static_cast<float>(screenHeight) - displayHeight_ / 2.0f;
    }

    void Draw() const {
        if (isHidden_) return;
        if (!isActive_) return;
        if (costumes_.empty()) return;

        const Costume& cos = costumes_[currentCostume_];

        Rectangle sourceRec{
            static_cast<float>(currentFrame_) * cos.frameWidth,
            0.0f,
            cos.frameWidth,
            cos.frameHeight
        };

        float rotationAngle = 0.0f;

        if (rotateWithDirection_) {
            // 真旋转模式：精灵图跟随 direction_ 旋转
            rotationAngle = direction_;
        } else {
            // 翻转模式：根据方向镜像纹理
            if (flipHorizontal_) {
                float normalizedDir = fmodf(direction_, 360.0f);
                if (normalizedDir < 0.0f) normalizedDir += 360.0f;
                if (normalizedDir > 90.0f && normalizedDir < 270.0f) {
                    sourceRec.width = -sourceRec.width;
                }
            }
        }

        const Rectangle destRec{ position_.x, position_.y, displayWidth_, displayHeight_ };
        const Vector2 origin{ displayWidth_ / 2.0f, displayHeight_ / 2.0f };

        DrawTexturePro(*cos.texture, sourceRec, destRec, origin, rotationAngle, colorEffect_);
    }

    void DrawBubble() const {
        if (isHidden_) return;
        if (!isActive_) return;
        if (bubbleText_.empty()) return;

        const Vector2 textSize = MeasureTextEx(GetFontDefault(), bubbleText_.c_str(), 20, 1);
        const Vector2 bubblePos{
            position_.x - textSize.x / 2.0f,
            position_.y - displayHeight_ / 2.0f - 30.0f
        };

        DrawRectangleRounded(
            { bubblePos.x - 10, bubblePos.y - 5, textSize.x + 20, textSize.y + 10 },
            0.5f, 10, LIGHTGRAY
        );

        if (isThinking_) {
            DrawCircle(
                static_cast<int>(bubblePos.x + textSize.x / 2.0f),
                static_cast<int>(bubblePos.y + textSize.y + 15.0f),
                5, LIGHTGRAY
            );
        }

        DrawText(
            bubbleText_.c_str(),
            static_cast<int>(bubblePos.x),
            static_cast<int>(bubblePos.y),
            20, DARKGRAY
        );
    }
};

} // namespace aur