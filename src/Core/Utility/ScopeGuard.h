#pragma once
#include <functional>

//! Calls function when object is destroyed
class ScopeGuard
{
public:
  ScopeGuard() : fn_(nullptr) {}

  template<class Callable>
  ScopeGuard(Callable&& fn) : fn_(std::forward<Callable>(fn))
  {
    set = true;
  }

  ScopeGuard(ScopeGuard&& other) noexcept : fn_(std::move(other.fn_))
  {
    other.fn_ = nullptr;
  }

  ~ScopeGuard() {
    // must not throw
    if (fn_) fn_();
  }

  ScopeGuard(const ScopeGuard&) = delete;
  void operator=(const ScopeGuard&) = delete;

private:
  std::function<void()> fn_;
  bool set = false;
};

#define CONCAT_(a, b) a ## b
#define CONCAT(a, b) CONCAT_(a,b)
#define RunEndOfScope(fn) ScopeGuard CONCAT(__defer__, __LINE__) = [&] ( ) { fn ; }
