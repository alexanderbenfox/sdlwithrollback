#pragma once
#include "Core/Utility/ScopeGuard.h"

struct DeferredFn
{
  static std::list<ScopeGuard> List;
};

struct DeferGuard
{
  DeferGuard() = default;
  ~DeferGuard() { DeferredFn::List.clear(); }
};

#define DEPAREN(X) ESC(ISH X)
#define ISH(...) ISH __VA_ARGS__
#define ESC(...) ESC_(__VA_ARGS__)
#define ESC_(...) VAN ## __VA_ARGS__
#define VANISH

#define RunOnDeferGuardDestroy(capture, code) DeferredFn::List.emplace_back(CONCAT([DEPAREN(capture)](), { code ; }));
