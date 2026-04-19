#include "EnactActionSystem.h"

void EnactAnimationActionSystem::PlayAnimation(EntityID entity, const std::string& animation, bool looped, float playSpeed, bool forceAnimRestart, bool facingRight)
{
  Animator& animator = ComponentArray<Animator>::Get().GetComponent(entity);
  RenderProperties& properties = ComponentArray<RenderProperties>::Get().GetComponent(entity);
  RenderComponent<RenderType>& renderer = ComponentArray<RenderComponent<RenderType>>::Get().GetComponent(entity);

  IAnimation* actionAnimation = animator.Play(animation, looped, playSpeed, forceAnimRestart);
  properties.horizontalFlip = !facingRight;
  if (actionAnimation)
    actionAnimation->ApplyInitialFrame(renderer, properties);
}
