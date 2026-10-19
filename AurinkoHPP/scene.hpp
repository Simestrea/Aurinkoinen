#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include <cstddef>
#include <aurinko/components/character/character.hpp>
#include "coroutine.hpp"

namespace aur {

class SceneManager {
private:
    int currentScene_{0};
    std::vector<Character*> characters_;
    std::vector<std::unique_ptr<Character>> owned_;   // 动态创建的克隆体

public:
    SceneManager() = default;
    ~SceneManager() = default;

    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;
    SceneManager(SceneManager&&) = delete;
    SceneManager& operator=(SceneManager&&) = delete;

    void Register(Character* c) {
        if (c) characters_.push_back(c);
    }

    // 动态创建一个克隆体，返回它的指针
    template<typename TaskType>
    Character* Spawn(const Character& source, int sceneId, TaskType task) {
        auto clone = std::make_unique<Character>(source);
        clone->SetScene(sceneId);
        clone->SetActive(sceneId == currentScene_);
        Character* ptr = clone.get();
        owned_.push_back(std::move(clone));
        characters_.push_back(ptr);
        detail::scheduler.Start(task(*ptr), sceneId);
        return ptr;
    }

    // 每帧清理已标记删除的克隆体
    void Cleanup() {
        std::vector<Character*> deleted;

        owned_.erase(
            std::remove_if(owned_.begin(), owned_.end(),
                [&deleted](const std::unique_ptr<Character>& c) {
                    if (c->IsMarkedForDeletion()) {
                        deleted.push_back(c.get());
                        return true;
                    }
                    return false;
                }),
            owned_.end()
        );

        characters_.erase(
            std::remove_if(characters_.begin(), characters_.end(),
                [&deleted](Character* c) {
                    return std::find(deleted.begin(), deleted.end(), c) != deleted.end();
                }),
            characters_.end()
        );
    }

    void Switch(int sceneId) {
        currentScene_ = sceneId;
        detail::scheduler.SetActiveGroup(sceneId);
        for (auto* c : characters_) {
            c->SetActive(c->GetScene() == sceneId);
        }
    }

    void Refresh() {
        detail::scheduler.SetActiveGroup(currentScene_);
        for (auto* c : characters_) {
            c->SetActive(c->GetScene() == currentScene_);
        }
    }

    int Current() const noexcept { return currentScene_; }
    std::size_t CharacterCount() const noexcept { return characters_.size(); }

    Character* Find(const std::string& name) {
        for (auto* c : characters_) {
            if (c && c->GetName() == name) return c;
        }
        return nullptr;
    }

    const std::vector<Character*>& GetAll() const noexcept { return characters_; }
};

// ==========================================
// 内部单例
// ==========================================
namespace detail {
    inline SceneManager sceneManager;
}

}