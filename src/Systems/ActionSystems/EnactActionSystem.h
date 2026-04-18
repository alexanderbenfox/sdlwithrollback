#pragma once
#include "Core/ECS/ISystem.h"

#include "Components/ActionComponents.h"
#include "Components/Animator.h"
#include "Components/RenderComponent.h"

struct EnactAnimationActionSystem : public ISystem<EnactActionComponent, AnimatedActionComponent, Animator, RenderProperties, RenderComponent<RenderType>>
{
  static void PlayAnimation(EntityID entity, const std::string& animation, bool looped, float playSpeed, bool forceAnimRestart, bool facingRight);
};
