#pragma once
#include "appearance.hpp"

namespace aur {

// ==========================================
// CharacterWithSound: sound effects
// ==========================================
class CharacterWithSound : public CharacterWithAppearance {
public:
    using CharacterWithAppearance::CharacterWithAppearance;

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
};

} // namespace aur
