#pragma once
#include "Systems/ISystem.h"
#include "Components/Animator.h"
#include "Components/StateComponents/AttackStateComponent.h"
#include "Components/StateComponent.h"

class AttackAnimationSystem : public ISystem<AttackStateComponent, Animator, Transform, StateComponent>
{
public:
  static void DoTick(float dt)
  {
    for(auto tuple : Tuples)
    {
      Animator* animator = std::get<Animator*>(tuple.second);
      Transform* transform = std::get<Transform*>(tuple.second);
      AttackStateComponent* atkState = std::get<AttackStateComponent*>(tuple.second);
      StateComponent* stateComp = std::get<StateComponent*>(tuple.second);

      if (atkState->lastFrame != animator->frame)
      {
        int frame = animator->frame;

        // update all in progress events now
        for (int i = 0; i < atkState->inProgressEvents.size(); i++)
        {
          AnimationEvent* evt = atkState->inProgressEvents[i];
          if (frame >= evt->GetEndFrame())
          {
            evt->EndEvent(transform);
            atkState->inProgressEvents.erase(atkState->inProgressEvents.begin() + i);
            i--;
          }
          else
          {
            evt->UpdateEvent(frame, transform, stateComp);
          }
        }

        // Checks if an event should be trigger this frame of animation and calls its callback if so
        std::vector<AnimationEvent>& potentialEvents = atkState->GetEventsStarting(frame);
        if (!potentialEvents.empty())
        {
          for (auto& evt : potentialEvents)
          {
            atkState->inProgressEvents.push_back(&evt);
            evt.TriggerEvent(transform, stateComp);
          }
        }

        // update the last frame updated
        atkState->lastFrame = animator->frame;
      }
    }
  }
};

class AnimationSystem : public ISystem<Animator, RenderComponent<RenderType>>
{
public:

  //
  static int GetNextFrameLooping(int framesToAdv, const int& currentFrame, const int& totalFrames)
  {
    return (currentFrame + framesToAdv) % totalFrames;
  }

  static int GetNextFrameOnce(int framesToAdv, const int& currentFrame, const int& totalFrames)
  {
    if ((currentFrame + framesToAdv) >= totalFrames)
    {
      return totalFrames - 1;
    }
    return currentFrame + framesToAdv;
  };

  static void DoTick(float dt)
  {
    for (auto tuple : Tuples)
    {
      Animator* animator = std::get<Animator*>(tuple.second);
      RenderComponent<RenderType>* renderer = std::get<RenderComponent<RenderType>*>(tuple.second);

      // if playing, do advance time and update frame
      if (animator->playing)
      {
        // when the animation is complete, do the listener callback
        // do this on the following frame so that the last frame of animation can still render
        if (auto* listener = animator->GetListener())
        {
          if (!animator->looping && animator->frame == (animator->GetCurrentAnimation().GetFrameCount() - 1))
            listener->OnAnimationComplete(animator->currentAnimationName);
        }

        if (animator->accumulatedTime >= secPerFrame)
        {
          int framesToAdv = (int)std::floor(animator->accumulatedTime / secPerFrame);

          // get next frame off of the type of anim it is
          int totalAnimFrames = animator->GetCurrentAnimation().GetFrameCount();

          int nextFrame = animator->looping ? GetNextFrameLooping(framesToAdv, animator->frame, totalAnimFrames)
            : GetNextFrameOnce(framesToAdv, animator->frame, totalAnimFrames);
          
          // frame has advanced, so we update the rect on source to reflect that
          if (nextFrame != animator->frame)
          {
            animator->frame = nextFrame;
            renderer->SetRenderResource(animator->GetCurrentAnimation().GetSheetTexture<RenderType>());
            renderer->sourceRect = animator->GetCurrentAnimation().GetFrameSrcRect(animator->frame);
          }

          // 
          animator->accumulatedTime -= (framesToAdv * secPerFrame);
        }
        // update accumulated time by dt and the animation speed modifier
        animator->accumulatedTime += (dt * animator->playSpeed);
      }
    }
  }
};
