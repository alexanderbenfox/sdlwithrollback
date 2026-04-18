#pragma once
#include "Core/ECS/IComponent.h"
#include "Components/Animator.h"
#include "Components/RenderComponent.h"

// Utility for playing an animation on an entity with render components.
// Not a system — just a static helper used by cutscenes, SFX, and editor previews.
struct EnactAnimationActionSystem
{
  static void PlayAnimation(EntityID entity, const std::string& animation, bool looped, float playSpeed, bool forceAnimRestart, bool facingRight);
};
