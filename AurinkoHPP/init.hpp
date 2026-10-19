#pragma once
#include "raylib.h"
#include <cmath>
#include <algorithm>


namespace aur {

class VirtualScreen {
public:
    VirtualScreen() = default;
    ~VirtualScreen() { Shutdown(); }

    VirtualScreen(const VirtualScreen&) = delete;
    VirtualScreen& operator=(const VirtualScreen&) = delete;
    VirtualScreen(VirtualScreen&&) = delete;
    VirtualScreen& operator=(VirtualScreen&&) = delete;

    void Init(int vWidth, int vHeight, const char* title, bool resizable = true) {
        virtualWidth_  = vWidth;
        virtualHeight_ = vHeight;

        if (resizable) SetConfigFlags(FLAG_WINDOW_RESIZABLE);

        InitWindow(virtualWidth_, virtualHeight_, title);

        // 初始纹理 = 窗口尺寸
        RecreateTarget(GetScreenWidth(), GetScreenHeight());

        SetWindowMinSize(320, static_cast<int>(320.0f * virtualHeight_ / virtualWidth_));
        SetExitKey(KEY_NULL);
        SetTargetFPS(60);
    }

    void Close() {
        Shutdown();
        CloseWindow();
    }

    void ExitFullscreenOnEsc() {
        if (IsKeyPressed(KEY_ESCAPE) && IsWindowFullscreen()) {
            ToggleFullscreen();
        }
    }

    void EnforceAspectRatio() {
        if (IsWindowMaximized()) return;
        if (IsWindowFullscreen()) return;
        if (!IsWindowResized()) return;

        const int currentW = GetScreenWidth();
        const int currentH = GetScreenHeight();
        const float targetAspect  = static_cast<float>(virtualWidth_) / static_cast<float>(virtualHeight_);
        const float currentAspect = static_cast<float>(currentW) / static_cast<float>(currentH);

        if (fabsf(currentAspect - targetAspect) < 0.005f) return;

        int targetW = currentW;
        int targetH = currentH;

        if (currentAspect > targetAspect) {
            targetW = static_cast<int>(currentH * targetAspect + 0.5f);
        } else {
            targetH = static_cast<int>(currentW / targetAspect + 0.5f);
        }

        SetWindowSize(targetW, targetH);
    }

    void ToggleFullscreen() { ::ToggleFullscreen(); }

    void Begin(Color clearColor = RAYWHITE) {
        // 窗口变了就重建纹理，保证 1:1
        const int cw = GetScreenWidth();
        const int ch = GetScreenHeight();
        if (cw != targetW_ || ch != targetH_ || target_.id == 0) {
            RecreateTarget(cw, ch);
        }

        BeginTextureMode(target_);
        ClearBackground(BLACK);

        // 计算等比缩放和偏移（虚拟画布居中放入纹理）
        const float scaleX = static_cast<float>(targetW_) / virtualWidth_;
        const float scaleY = static_cast<float>(targetH_) / virtualHeight_;
        scale_   = std::min(scaleX, scaleY);
        offsetX_ = (static_cast<float>(targetW_) - virtualWidth_  * scale_) * 0.5f;
        offsetY_ = (static_cast<float>(targetH_) - virtualHeight_ * scale_) * 0.5f;

        // 在可视区域填用户清屏色（周围留黑边）
        DrawRectangle(
            static_cast<int>(offsetX_),
            static_cast<int>(offsetY_),
            static_cast<int>(virtualWidth_  * scale_ + 0.5f),
            static_cast<int>(virtualHeight_ * scale_ + 0.5f),
            clearColor
        );

        // Camera2D：虚拟坐标 → 纹理像素
        camera_.offset   = { offsetX_, offsetY_ };
        camera_.target   = { 0.0f, 0.0f };
        camera_.rotation = 0.0f;
        camera_.zoom     = scale_;

        BeginMode2D(camera_);
    }

    void End() {
        EndMode2D();
        EndTextureMode();

        BeginDrawing();
        ClearBackground(BLACK);

        // 纹理 1:1 贴回窗口
        const Rectangle sourceRec{
            0.0f,
            0.0f,
            static_cast<float>(target_.texture.width),
            -static_cast<float>(target_.texture.height)
        };
        const Rectangle destRec{
            0.0f,
            0.0f,
            static_cast<float>(targetW_),
            static_cast<float>(targetH_)
        };
        DrawTexturePro(target_.texture, sourceRec, destRec,
                       { 0.0f, 0.0f }, 0.0f, WHITE);

        EndDrawing();
    }

    Vector2 GetVirtualMousePosition() const {
        const Vector2 m = ::GetMousePosition();
        return {
            (m.x - offsetX_) / scale_,
            (m.y - offsetY_) / scale_
        };
    }

    int  GetVirtualWidth()  const noexcept { return virtualWidth_; }
    int  GetVirtualHeight() const noexcept { return virtualHeight_; }
    bool IsFullscreen()     const noexcept { return ::IsWindowFullscreen(); }

private:
    void RecreateTarget(int w, int h) {
        if (w < 1) w = 1;
        if (h < 1) h = 1;

        if (target_.id != 0) {
            UnloadRenderTexture(target_);
            target_ = RenderTexture2D{};
        }
        target_ = LoadRenderTexture(w, h);
        SetTextureFilter(target_.texture, TEXTURE_FILTER_BILINEAR);

        targetW_ = w;
        targetH_ = h;
    }

    void Shutdown() {
        if (target_.id != 0) {
            UnloadRenderTexture(target_);
            target_ = RenderTexture2D{};
        }
    }

    RenderTexture2D target_{};
    Camera2D camera_{};
    int   virtualWidth_{800};
    int   virtualHeight_{450};
    int   targetW_{0};
    int   targetH_{0};
    float scale_{1.0f};
    float offsetX_{0.0f};
    float offsetY_{0.0f};
};

namespace detail {
    inline VirtualScreen virtualScreen;
}

}