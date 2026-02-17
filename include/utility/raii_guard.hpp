#pragma once

#include <functional>
#include <memory>

namespace toyc::utility {

// Generic RAII guard that executes cleanup function on scope exit
template <typename T>
class ScopeGuard {
public:
    ScopeGuard(std::function<void()> cleanup) : cleanup_(cleanup), dismissed_(false) {}

    ~ScopeGuard() {
        if (!dismissed_) {
            cleanup_();
        }
    }

    // Prevent copying
    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;

    // Allow moving
    ScopeGuard(ScopeGuard&& other) noexcept : cleanup_(std::move(other.cleanup_)), dismissed_(other.dismissed_) {
        other.dismissed_ = true;
    }

    // Dismiss the guard, preventing cleanup
    void dismiss() { dismissed_ = true; }

private:
    std::function<void()> cleanup_;
    bool dismissed_;
};

// Helper function to create ScopeGuard
template <typename F>
auto makeScopeGuard(F&& cleanup) {
    return ScopeGuard<F>(std::forward<F>(cleanup));
}

}  // namespace toyc::utility
