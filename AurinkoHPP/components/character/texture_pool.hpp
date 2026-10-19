#pragma once
#include "raylib.h"
#include <memory>
#include <string>
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

namespace detail {
    inline TexturePool texturePool;
}

} // namespace aur
