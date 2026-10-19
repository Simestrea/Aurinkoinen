#pragma once
#include <coroutine>
#include <vector>
#include <memory>
#include <algorithm>
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

    std::vector<PendingEntry> pending_;
    std::vector<RunningEntry> running_;
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

    void Update(float deltaTime) {
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

}