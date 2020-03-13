#pragma once
#include "Systems/ISystem.h"
#include "Components/Animator.h"

//!
const float secPerFrame = 1.0f / animation_fps;

class AnimationSystem : public ISystem<AnimationRenderer, Transform>
{
public:
  static void DoTick(float dt)
  {
    for (auto tuple : Tuples)
    {
      AnimationRenderer* renderer = std::get<AnimationRenderer*>(tuple.second);
      Transform* transform = std::get<Transform*>(tuple.second);
      // if playing, do advance time and update frame
      if (renderer->IsPlaying())
      {
        renderer->PlayTime() += dt;
        if (renderer->PlayTime() >= secPerFrame)
        {
          int framesToAdv = (int)std::floor(renderer->PlayTime() / secPerFrame);

          // get next frame off of the type of anim it is
          int nextFrame = renderer->GetNextFrame()(framesToAdv);

          if (nextFrame != renderer->GetCurrentFrame())
          {
            renderer->GetCurrentFrame() = nextFrame;
            renderer->GetCurrentAnimation().CheckEvents(nextFrame, transform->position.x + renderer->GetDisplayOffset().x, transform->position.y + renderer->GetDisplayOffset().y);
          }

          // 
          renderer->PlayTime() -= (framesToAdv * secPerFrame);

          // when the animation is complete, do the listener callback
          if (renderer->GetListener())
          {
            if (!renderer->IsLooping() && renderer->GetCurrentFrame() == (renderer->GetCurrentAnimation().GetFrameCount() - 1))
              renderer->GetListener()->OnAnimationComplete(renderer->GetAnimationName());
          }
        }
      }
    }
  }

  static void PostUpdate()
  {
    for (auto tuple : Tuples)
    {
      // get a display op to set draw parameters
      auto displayOp = ResourceManager::Get().GetAvailableOp();

      AnimationRenderer* renderer = std::get<AnimationRenderer*>(tuple.second);
      Transform* transform = std::get<Transform*>(tuple.second);

      renderer->GetDisplayable()->SetOp(*transform, renderer->GetSourceRect(),
        renderer->GetDisplayOffset(), renderer->GetFlip(), displayOp);
    }
  }
};
