#pragma once
#include "Core/ECS/ISystem.h"
#include "Components/Animator.h"
#include "Components/Hurtbox.h"
#include "Components/StateComponent.h"

#include "AssetManagement/IAnimation.h"
#include "Managers/AnimationCollectionManager.h"

//! Updates hurtbox rect each frame from the current animation's per-frame hurtbox data.
//! Runs after AnimationSystem + MoveSystem so the frame and position are up to date.
class HurtboxUpdateSystem : public ISystem<Animator, Hurtbox, Transform, StateComponent>
{
public:
  static void DoTick(float dt)
  {
    PROFILE_FUNCTION();
    for (const EntityID& entity : Registered)
    {
      Animator& animator = ComponentArray<Animator>::Get().GetComponent(entity);
      Hurtbox& hurtbox = ComponentArray<Hurtbox>::Get().GetComponent(entity);
      Transform& transform = ComponentArray<Transform>::Get().GetComponent(entity);
      StateComponent& state = ComponentArray<StateComponent>::Get().GetComponent(entity);

      IAnimation* animation = GAnimArchive.GetAnimationData(animator.animCollectionID, animator.currentAnimationName);
      if (!animation || !animation->HasHurtboxData())
        continue;

      Rect<double> frameHurtbox = animation->GetFrameHurtbox(animator.frame);
      if (frameHurtbox.Area() == 0)
        continue;

      // Scale from source-pixel space to game space
      auto scaling = animation->GetRenderScaling();
      frameHurtbox.beg *= scaling;
      frameHurtbox.end *= scaling;

      // Get the offset from transform position to sprite frame origin
      Vector2<float> dataOffset = animation->GetDataOffset();

      // Position hurtbox relative to transform (same logic as MoveDataBoxAroundTransform)
      Vector2<double> relativeToCenter = frameHurtbox.GetCenter() - (Vector2<double>)dataOffset;
      if (!state.onLeftSide)
        relativeToCenter.x *= -1.0;

      hurtbox.rect = Rect<double>(0, 0, transform.scale.x * frameHurtbox.Width(), transform.scale.y * frameHurtbox.Height());
      hurtbox.rect.CenterOnPoint((Vector2<double>)transform.position + transform.scale * relativeToCenter);
    }
  }
};
