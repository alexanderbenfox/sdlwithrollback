#include "EnactActionSystem.h"
#include "Managers/AnimationCollectionManager.h"

void EnactAnimationActionSystem::PlayAnimation(EntityID entity, const std::string& animation, bool looped, float playSpeed, bool forceAnimRestart, bool facingRight)
{
  Animator& animator = ComponentArray<Animator>::Get().GetComponent(entity);
  RenderProperties& properties = ComponentArray<RenderProperties>::Get().GetComponent(entity);
  RenderComponent<RenderType>& renderer = ComponentArray<RenderComponent<RenderType>>::Get().GetComponent(entity);

  Animation* actionAnimation = animator.Play(animation, looped, playSpeed, forceAnimRestart);
  properties.horizontalFlip = !facingRight;
  properties.anchor = actionAnimation->GetAnchorForAnimFrame(0).first;
  properties.offset = actionAnimation->GetAnchorForAnimFrame(0).second;
  properties.renderScaling = actionAnimation->GetRenderScaling();

  renderer.SetRenderResource(actionAnimation->GetSheetTexture<RenderType>());
  renderer.sourceRect = actionAnimation->GetFrameSrcRect(0);
}
