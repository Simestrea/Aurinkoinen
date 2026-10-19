#!/usr/bin/env python3
# -*- coding: gbk -*-
"""
给 aurinko 加动画系统。

创建：
  aurinko/easing.hpp              缓动函数

patch：
  aurinko/coroutine.hpp           Tween / Parallel + 调度器 tick 支持
  aurinko/components/character/appearance.hpp
                                  FadeIn / FadeOut / ScaleTo / MoveTo
  aurinko/aurinko.hpp             加 include

用法：python add_animation.py [--dry-run]
"""

import sys
from pathlib import Path

DRY_RUN = "--dry-run" in sys.argv
ENC = "gbk"
HERE = Path(__file__).resolve().parent


# ============================================================
# 定位 aurinko 库
# ============================================================
def find_lib():
    if (HERE / "aurinko.hpp").exists():
        return HERE
    if "--lib" in sys.argv:
        i = sys.argv.index("--lib")
        if i + 1 < len(sys.argv):
            p = Path(sys.argv[i + 1]).resolve()
            if (p / "aurinko.hpp").exists():
                return p
            if (p / "aurinko" / "aurinko.hpp").exists():
                return p / "aurinko"
    for c in [HERE / "aurinko", HERE.parent / "aurinko",
              HERE.parent / "Aurinko" / "aurinko"]:
        if (c / "aurinko.hpp").exists():
            return c
    return None


LIB = find_lib()
if LIB is None:
    print("错误: 找不到 aurinko 库（含 aurinko.hpp）")
    print("  把脚本放到 aurinko/ 目录里，或用 --lib <path> 指定")
    sys.exit(1)

print(f"aurinko 库目录: {LIB}")
print()


# ============================================================
# easing.hpp（新文件）
# ============================================================
EASING_HPP = r'''#pragma once
// ==========================================
// aur::ease —— 缓动函数
//
// 用法：
//   co_await c.MoveTo(300, 200, 0.8f, aur::ease::OutQuad);
//   co_await c.FadeIn(0.5f, aur::ease::OutCubic);
//
// 所有函数签名：float(float t)，t ∈ [0, 1]，返回缓动后的值
// ==========================================
#include <cmath>

namespace aur::ease {

using EaseFn = float(*)(float);

// ---------- 线性 ----------
inline float Linear(float t) { return t; }

// ---------- Quad ----------
inline float InQuad(float t)  { return t * t; }
inline float OutQuad(float t) { return 1.0f - (1.0f - t) * (1.0f - t); }
inline float InOutQuad(float t) {
    return t < 0.5f ? 2.0f * t * t
                    : 1.0f - 2.0f * (1.0f - t) * (1.0f - t);
}

// ---------- Cubic ----------
inline float InCubic(float t)  { return t * t * t; }
inline float OutCubic(float t) {
    float f = 1.0f - t;
    return 1.0f - f * f * f;
}
inline float InOutCubic(float t) {
    return t < 0.5f
        ? 4.0f * t * t * t
        : 1.0f - 4.0f * (1.0f - t) * (1.0f - t) * (1.0f - t);
}

// ---------- Quart ----------
inline float InQuart(float t)  { return t * t * t * t; }
inline float OutQuart(float t) {
    float f = 1.0f - t;
    return 1.0f - f * f * f * f;
}

// ---------- Quint ----------
inline float InQuint(float t)  { return t * t * t * t * t; }
inline float OutQuint(float t) {
    float f = 1.0f - t;
    return 1.0f - f * f * f * f * f;
}

// ---------- Sine ----------
inline float InSine(float t) {
    return 1.0f - std::cos(t * 3.14159265358979f * 0.5f);
}
inline float OutSine(float t) {
    return std::sin(t * 3.14159265358979f * 0.5f);
}
inline float InOutSine(float t) {
    return -(std::cos(3.14159265358979f * t) - 1.0f) * 0.5f;
}

// ---------- Back（回弹） ----------
inline float OutBack(float t) {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    float f = t - 1.0f;
    return 1.0f + c3 * f * f * f + c1 * f * f;
}
inline float InBack(float t) {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    return c3 * t * t * t - c1 * t * t;
}

// ---------- Elastic（弹性） ----------
inline float OutElastic(float t) {
    if (t <= 0.0f) return 0.0f;
    if (t >= 1.0f) return 1.0f;
    const float c4 = 2.0f * 3.14159265358979f / 3.0f;
    return std::pow(2.0f, -10.0f * t)
         * std::sin((t * 10.0f - 0.75f) * c4) + 1.0f;
}

// ---------- Bounce（弹跳） ----------
inline float OutBounce(float t) {
    const float n1 = 7.5625f;
    const float d1 = 2.75f;
    if (t < 1.0f / d1) { return n1 * t * t; }
    if (t < 2.0f / d1) { t -= 1.5f / d1; return n1 * t * t + 0.75f; }
    if (t < 2.5f / d1) { t -= 2.25f / d1; return n1 * t * t + 0.9375f; }
    t -= 2.625f / d1;
    return n1 * t * t + 0.984375f;
}

} // namespace aur::ease
'''


# ============================================================
# 读写
# ============================================================
def read(p):
    try:
        return p.read_text(encoding=ENC)
    except UnicodeDecodeError:
        return p.read_text(encoding="utf-8")


def write(p, t):
    p.write_text(t, encoding=ENC, newline="\n")


def patch(path, patches):
    if not path.exists():
        print(f"  [skip] 找不到 {path.name}")
        return 0
    text = read(path)
    orig = text
    n = 0
    for marker, old, new in patches:
        if old not in text:
            print(f"  [warn] {path.name}: 未找到 '{marker[:44]}'")
            continue
        text = text.replace(old, new, 1)
        n += 1
        print(f"  [ok] {path.name}: {marker[:44]}")
    if text != orig:
        if DRY_RUN:
            print(f"  [dry] {path.name} 未写盘")
        else:
            write(path, text)
            print(f"  [write] {path.name}")
    return n


# ============================================================
# 步骤 1：easing.hpp
# ============================================================
def step_easing():
    print("[1/4] 创建 easing.hpp")
    p = LIB / "easing.hpp"
    if p.exists():
        print(f"  [skip] 已存在")
        return
    if DRY_RUN:
        print(f"  [dry] 将写 {p.relative_to(LIB)}")
    else:
        write(p, EASING_HPP)
        print(f"  [write] {p.relative_to(LIB)}")


# ============================================================
# 步骤 2：coroutine.hpp
# ============================================================
def step_coroutine():
    print("[2/4] patch coroutine.hpp")
    p = LIB / "coroutine.hpp"
    patches = [

    # include <functional>
    ("include functional",
     "#pragma once\n#include <coroutine>\n#include <vector>\n#include <memory>\n#include <algorithm>\n#include <cstdlib>\n",
     "#pragma once\n#include <coroutine>\n#include <vector>\n#include <memory>\n#include <algorithm>\n#include <functional>\n#include <initializer_list>\n#include <cstdlib>\n"),

    # TickEntry 结构 + ticks_ 字段
    ("TickEntry",
     """    struct RunningEntry {
        std::coroutine_handle<> handle;
        int groupId;
    };

    std::vector<PendingEntry> pending_;
    std::vector<RunningEntry> running_;
    int activeGroup_{0};
""",
     """    struct RunningEntry {
        std::coroutine_handle<> handle;
        int groupId;
    };

    struct TickEntry {
        std::function<bool(float)> fn;   // 返回 true = 完成，会被移除
        std::coroutine_handle<>    waiter; // 完成后要 resume 的协程（可空）
    };

    std::vector<PendingEntry> pending_;
    std::vector<RunningEntry> running_;
    std::vector<TickEntry>    ticks_;
    int activeGroup_{0};
"""),

    # AddTick 方法
    ("AddTick",
     """    void Register(std::coroutine_handle<> h, float seconds) {
        int group = activeGroup_;
        for (auto& entry : running_) {
            if (entry.handle == h) {
                group = entry.groupId;
                break;
            }
        }
        pending_.push_back({ h, seconds, group });
    }
""",
     """    void Register(std::coroutine_handle<> h, float seconds) {
        int group = activeGroup_;
        for (auto& entry : running_) {
            if (entry.handle == h) {
                group = entry.groupId;
                break;
            }
        }
        pending_.push_back({ h, seconds, group });
    }

    // 每帧 tick 回调。fn 返回 true 表示完成（自动移除）
    // 如果 waiter 非空，会在完成时 resume
    void AddTick(std::function<bool(float)> fn,
                 std::coroutine_handle<> waiter = {}) {
        ticks_.push_back({ std::move(fn), waiter });
    }
"""),

    # Update 开头加 ticks 处理
    ("Update ticks",
     """    void Update(float deltaTime) {
        std::vector<std::coroutine_handle<>> toResume;
        toResume.reserve(pending_.size());
""",
     """    void Update(float deltaTime) {
        // ---------- 每帧动画 tick ----------
        if (!ticks_.empty()) {
            std::vector<std::coroutine_handle<>> tickDone;
            for (auto it = ticks_.begin(); it != ticks_.end(); ) {
                bool done = it->fn(deltaTime);
                if (done) {
                    if (it->waiter) tickDone.push_back(it->waiter);
                    it = ticks_.erase(it);
                } else {
                    ++it;
                }
            }
            for (auto h : tickDone) h.resume();
        }

        // ---------- 等待队列 ----------
        std::vector<std::coroutine_handle<>> toResume;
        toResume.reserve(pending_.size());
"""),

    # Clear 里加 ticks_ 清空
    ("Clear ticks",
     """    void Clear() noexcept {
        pending_.clear();
        for (auto& e : running_) {
            if (e.handle) e.handle.destroy();
        }
        running_.clear();
    }
""",
     """    void Clear() noexcept {
        pending_.clear();
        ticks_.clear();
        for (auto& e : running_) {
            if (e.handle) e.handle.destroy();
        }
        running_.clear();
    }
"""),

    # 文件末尾加 Tween / Parallel
    ("Tween / Parallel",
     """template<typename TaskType>
inline void Fire(TaskType task) {
    detail::scheduler.Start(std::move(task),
                            detail::scheduler.GetActiveGroup());
}
""",
     """template<typename TaskType>
inline void Fire(TaskType task) {
    detail::scheduler.Start(std::move(task),
                            detail::scheduler.GetActiveGroup());
}

// ==========================================
// 动画 awaitable
// ==========================================

// 一段时长内，每帧回调 onFrame(t)
// t ∈ [0, 1]，已过缓动
//
// 用法：
//   co_await aur::Tween{ 0.5f, [&](float t){ c.SetColorEffect(255*t); } };
//   co_await aur::Tween{ 0.8f, ..., aur::ease::OutQuad };
struct Tween {
    float duration = 0.0f;
    std::function<void(float)> onFrame;
    float (*ease)(float) = nullptr;   // 空 = 线性

    bool await_ready() const noexcept { return duration <= 0.0f; }

    void await_suspend(std::coroutine_handle<> h) const {
        if (!onFrame) { h.resume(); return; }

        auto cb          = onFrame;
        float dur        = duration;
        float (*ease_)(float) = ease;

        detail::scheduler.AddTick(
            [cb, dur, ease_, elapsed = 0.0f](float dt) mutable -> bool {
                elapsed += dt;
                float t = (dur > 0.0f)
                          ? (elapsed >= dur ? 1.0f : elapsed / dur)
                          : 1.0f;
                float e = ease_ ? ease_(t) : t;
                cb(e);
                return t >= 1.0f;
            },
            h);
    }

    void await_resume() const noexcept {}
};

// 并行等待多个 Tween，全部完成才恢复
//
// 用法：
//   co_await aur::Parallel({
//       aur::Tween{ 0.3f, [&](float t){ c.SetColorEffect(255*t); } },
//       aur::Tween{ 0.3f, [&](float t){ c.SetScale(0.5f + t * 0.5f); } },
//   });
struct Parallel {
    std::vector<Tween> tweens;

    Parallel(std::initializer_list<Tween> list) : tweens(list) {}

    bool await_ready() const noexcept { return tweens.empty(); }

    void await_suspend(std::coroutine_handle<> h) const {
        int total = 0;
        for (auto& tw : tweens) if (tw.onFrame) ++total;
        if (total == 0) { h.resume(); return; }

        auto counter = std::make_shared<int>(total);
        auto hc = h;

        for (auto& tw : tweens) {
            if (!tw.onFrame) continue;

            auto cb          = tw.onFrame;
            float dur        = tw.duration;
            float (*ease_)(float) = tw.ease;

            detail::scheduler.AddTick(
                [cb, dur, ease_, counter, hc, elapsed = 0.0f]
                (float dt) mutable -> bool {
                    elapsed += dt;
                    float t = (dur > 0.0f)
                              ? (elapsed >= dur ? 1.0f : elapsed / dur)
                              : 1.0f;
                    float e = ease_ ? ease_(t) : t;
                    cb(e);
                    if (t >= 1.0f) {
                        if (--(*counter) == 0) hc.resume();
                        return true;
                    }
                    return false;
                },
                {});
        }
    }

    void await_resume() const noexcept {}
};
"""),
    ]
    return patch(p, patches)


# ============================================================
# 步骤 3：character/appearance.hpp
# ============================================================
def step_appearance():
    print("[3/4] patch character/appearance.hpp")
    p = LIB / "components" / "character" / "appearance.hpp"
    patches = [

    ("include coroutine + easing",
     '#pragma once\n#include "motion.hpp"\n',
     '#pragma once\n#include "motion.hpp"\n#include <aurinko/coroutine.hpp>\n#include <aurinko/easing.hpp>\n'),

    ("动画方法",
     """    void GoToFront()                 { zIndex_ = 999; }
    void GoForwardLayers(int layers) { zIndex_ += layers; }
};
""",
     """    void GoToFront()                 { zIndex_ = 999; }
    void GoForwardLayers(int layers) { zIndex_ += layers; }

    // ==========================================
    // 动画（等待完成的协程动作）
    // ==========================================
    // 用法：
    //   co_await c.FadeIn(0.5f);
    //   co_await c.MoveTo(300, 200, 0.8f, aur::ease::OutQuad);
    //   co_await aur::Parallel({
    //       c.FadeIn(0.3f),
    //       c.ScaleTo(1.5f, 0.3f),
    //   });

    // 淡入：透明度 0 → 255
    Tween FadeIn(float duration = 0.3f, float (*ease)(float) = nullptr) {
        return Tween{ duration, [this](float t) {
            SetColorEffect(255.0f * t);
        }, ease };
    }

    // 淡出：透明度 → 0
    Tween FadeOut(float duration = 0.3f, float (*ease)(float) = nullptr) {
        return Tween{ duration, [this](float t) {
            SetColorEffect(255.0f * (1.0f - t));
        }, ease };
    }

    // 缩放到指定倍数
    Tween ScaleTo(float target, float duration = 0.3f,
                  float (*ease)(float) = nullptr) {
        float start = scale_;
        return Tween{ duration, [this, start, target](float t) {
            scale_ = start + (target - start) * t;
            displayWidth_  = frameWidth_  * scale_;
            displayHeight_ = frameHeight_ * scale_;
        }, ease };
    }

    // 移动到指定坐标
    Tween MoveTo(float x, float y, float duration = 0.5f,
                 float (*ease)(float) = nullptr) {
        Vector2 start  = position_;
        Vector2 target = { x, y };
        return Tween{ duration, [this, start, target](float t) {
            position_ = Vector2Lerp(start, target, t);
            isGliding_ = false;
        }, ease };
    }
};
"""),
    ]
    return patch(p, patches)


# ============================================================
# 步骤 4：aurinko.hpp
# ============================================================
def step_aurinko():
    print("[4/4] patch aurinko.hpp")
    p = LIB / "aurinko.hpp"
    if "aurinko/easing.hpp" in read(p):
        print("  [already] 已包含 easing.hpp")
        return 0
    patches = [
    ("include easing",
     "#include <aurinko/init.hpp>\n#include <aurinko/coroutine.hpp>\n",
     "#include <aurinko/init.hpp>\n#include <aurinko/coroutine.hpp>\n#include <aurinko/easing.hpp>\n"),
    ]
    return patch(p, patches)


# ============================================================
def main():
    print("=" * 56)
    print("给 aurinko 添加动画系统")
    print("=" * 56)
    print()

    step_easing()
    print()
    step_coroutine()
    print()
    step_appearance()
    print()
    step_aurinko()

    print()
    if DRY_RUN:
        print("dry-run 完成，去掉 --dry-run 再跑一次。")
        return

    print("完成。用起来像这样：")
    print()
    print("  // 淡入")
    print("  co_await c.FadeIn(0.5f);")
    print()
    print("  // 移动到 (300, 200)，0.8 秒，缓出")
    print("  co_await c.MoveTo(300, 200, 0.8f, aur::ease::OutQuad);")
    print()
    print("  // 缩放到 1.5 倍，带回弹")
    print("  co_await c.ScaleTo(1.5f, 0.4f, aur::ease::OutBack);")
    print()
    print("  // 并行：淡入 + 放大，同时进行")
    print("  co_await aur::Parallel({")
    print("      c.FadeIn(0.3f),")
    print("      c.ScaleTo(1.5f, 0.3f, aur::ease::OutBack),")
    print("  });")
    print()
    print("  // 淡出 + 消失")
    print("  co_await c.FadeOut(0.4f);")
    print("  c.Hide();")
    print()
    print("可用缓动函数（aur::ease::）：")
    print("  Linear  InQuad  OutQuad  InOutQuad")
    print("  InCubic OutCubic InOutCubic")
    print("  InQuart OutQuart  InQuint OutQuint")
    print("  InSine  OutSine   InOutSine")
    print("  InBack  OutBack   OutElastic  OutBounce")


if __name__ == "__main__":
    main()