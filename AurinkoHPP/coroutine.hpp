#pragma once
#include <coroutine>
#include <vector>
#include <memory>
#include <algorithm>
#include <functional>
#include <initializer_list>
#include <cstdlib>


namespace aur {

class CoroutineScheduler;

struct WaitFor {
    float seconds{0.0f};

    bool await_ready() const noexcept { return seconds <= 0.0f; }
    void await_suspend(std::coroutine_handle<> h) const noexcept;
    void await_resume() const noexcept {}
};

struct WaitNextFrame {
    bool await_ready() const noexcept { return false; }
    void await_suspend(std::coroutine_handle<> h) const noexcept;
    void await_resume() const noexcept {}
};

namespace detail {
    struct TaskTag {};
    struct CloneTag {};
    struct ActionTag {};

    template<typename Tag>
    class TaskBase {
    private:
        std::coroutine_handle<> handle_{};

    public:
        struct promise_type {
            TaskBase get_return_object() {
                return TaskBase{ std::coroutine_handle<promise_type>::from_promise(*this) };
            }
            std::suspend_always initial_suspend() noexcept { return {}; }
            std::suspend_always final_suspend() noexcept { return {}; }
            void return_void() noexcept {}
            void unhandled_exception() { std::terminate(); }
        };

        TaskBase() noexcept = default;
        explicit TaskBase(std::coroutine_handle<> h) noexcept : handle_{ h } {}

        TaskBase(const TaskBase&) = delete;
        TaskBase& operator=(const TaskBase&) = delete;

        TaskBase(TaskBase&& other) noexcept : handle_{ other.handle_ } {
            other.handle_ = nullptr;
        }

        TaskBase& operator=(TaskBase&& other) noexcept {
            if (this != &other) {
                if (handle_) handle_.destroy();
                handle_ = other.handle_;
                other.handle_ = nullptr;
            }
            return *this;
        }

        ~TaskBase() {
            if (handle_) handle_.destroy();
        }

        void Resume() noexcept {
            if (handle_ && !handle_.done()) handle_.resume();
        }

        bool Done() const noexcept {
            return !handle_ || handle_.done();
        }

        bool IsValid() const noexcept {
            return static_cast<bool>(handle_);
        }

        std::coroutine_handle<> GetHandle() const noexcept {
            return handle_;
        }

        // 把句柄所有权交给调度器
        void Detach() noexcept {
            handle_ = nullptr;
        }
    };
}

using Task = detail::TaskBase<detail::TaskTag>;
using Clone = detail::TaskBase<detail::CloneTag>;
using Action = detail::TaskBase<detail::ActionTag>;

class CoroutineScheduler {
private:
    struct PendingEntry {
        std::coroutine_handle<> handle;
        float timer;
        int groupId;
    };

    struct RunningEntry {
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

public:
    CoroutineScheduler() = default;
    ~CoroutineScheduler() { Clear(); }

    CoroutineScheduler(const CoroutineScheduler&) = delete;
    CoroutineScheduler& operator=(const CoroutineScheduler&) = delete;
    CoroutineScheduler(CoroutineScheduler&&) = delete;
    CoroutineScheduler& operator=(CoroutineScheduler&&) = delete;

    void SetActiveGroup(int g) noexcept { activeGroup_ = g; }
    int GetActiveGroup() const noexcept { return activeGroup_; }

    template<typename TaskType>
    void Start(TaskType task, int groupId = 0) {
        auto h = task.GetHandle();
        running_.push_back({ h, groupId });
        task.Detach();

        if (h && !h.done()) h.resume();
    }

    void Register(std::coroutine_handle<> h, float seconds) {
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

    void Update(float deltaTime) {
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

        for (auto it = pending_.begin(); it != pending_.end(); ) {
            if (it->groupId != activeGroup_) {
                ++it;
                continue;
            }

            it->timer -= deltaTime;
            if (it->timer <= 0.0f) {
                toResume.push_back(it->handle);
                it = pending_.erase(it);
            } else {
                ++it;
            }
        }

        for (auto h : toResume) {
            h.resume();
        }

        for (auto it = running_.begin(); it != running_.end(); ) {
            if (!it->handle || it->handle.done()) {
                if (it->handle) it->handle.destroy();
                it = running_.erase(it);
            } else {
                ++it;
            }
        }
    }

    void Clear() noexcept {
        pending_.clear();
        ticks_.clear();
        for (auto& e : running_) {
            if (e.handle) e.handle.destroy();
        }
        running_.clear();
    }

    std::size_t GetPendingCount() const noexcept { return pending_.size(); }
    std::size_t GetRunningCount() const noexcept { return running_.size(); }
    bool IsEmpty() const noexcept { return pending_.empty() && running_.empty(); }
};

// ==========================================
// 内部单例
// ==========================================
namespace detail {
    inline CoroutineScheduler scheduler;
}

inline void WaitFor::await_suspend(std::coroutine_handle<> h) const noexcept {
    detail::scheduler.Register(h, seconds);
}

inline void WaitNextFrame::await_suspend(std::coroutine_handle<> h) const noexcept {
    detail::scheduler.Register(h, 0.0f);
}

// ==========================================
// Fire: fire-and-forget
//
// 把一个 Action / Task 挂到当前场景的调度器上，立刻返回。
// 调用者不会等它跑完。
//
// 用法:
//   aur::Fire(BobDance(*bob));
// ==========================================
template<typename TaskType>
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

}