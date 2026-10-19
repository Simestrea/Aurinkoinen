#pragma once
#include "raylib.h"
#include "raymath.h"
#include "texture_pool.hpp"
#include <aurinko/init.hpp>
#include <cmath>
#include <string>
#include <vector>
#include <memory>

namespace aur {

// ==========================================
// CharacterCore: data + lifecycle + name/scene + rotation mode
// ==========================================
class CharacterCore {
protected:
    struct Costume {
        std::shared_ptr<Texture2D> texture{};
        std::string name{};
        int   frameCount{10};
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

    void MoveFrom(CharacterCore&& other) noexcept {
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
    CharacterCore() = default;

    CharacterCore(const std::string& name, Vector2 startPos, float startScale)
        : name_{name}, position_{startPos}, scale_{startScale} {}

    ~CharacterCore() = default;

    CharacterCore(const CharacterCore&) = default;
    CharacterCore& operator=(const CharacterCore&) = default;

    CharacterCore(CharacterCore&& other) noexcept { MoveFrom(std::move(other)); }
    CharacterCore& operator=(CharacterCore&& other) noexcept {
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

    const std::string& GetName() const noexcept { return name_; }
    void SetScene(int id) noexcept { sceneId_ = id; }
    int  GetScene() const noexcept { return sceneId_; }
    void SetActive(bool a) noexcept { isActive_ = a; }
    bool IsActive() const noexcept { return isActive_; }

    void SetRotateWithDirection(bool on) { rotateWithDirection_ = on; }
    bool IsRotateWithDirection() const noexcept { return rotateWithDirection_; }
    void SetFlipHorizontal(bool on) { flipHorizontal_ = on; }
    bool IsFlipHorizontal() const noexcept { return flipHorizontal_; }
};

} // namespace aur
