#pragma once
#include "core.hpp"

namespace aur {

// ==========================================
// CharacterWithCostume: costume management
// ==========================================
class CharacterWithCostume : public CharacterCore {
public:
    using CharacterCore::CharacterCore;

    void AddCostume(const char* texturePath, int frameCount,
                    float frameSpeed = 0.15f) {
        Costume cos;
        cos.texture = detail::texturePool.Load(texturePath);
        cos.name = texturePath;
        cos.frameCount = frameCount;
        cos.frameSpeed = frameSpeed;
        cos.frameWidth  = static_cast<float>(cos.texture->width) / frameCount;
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
};

} // namespace aur
