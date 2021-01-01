#pragma once
#include "Core/ECS/ISystem.h"
#include "Components/Animator.h"
#include "Components/RenderComponent.h"
#include "Components/StateComponents/AttackStateComponent.h"
#include "Components/StateComponent.h"

#include "Managers/AnimationCollectionManager.h"

class AttackAnimationSystem : public ISystem<AttackStateComponent, Animator, Transform, StateComponent>
{
public:
  static void DoTick(float dt)
  {
    for(const EntityID& entity : Registered)
    {
      PROFILE_FUNCTION();
      Animator& animator = ComponentArray<Animator>::Get().GetComponent(entity);
      Transform& transform = ComponentArray<Transform>::Get().GetComponent(entity);
      AttackStateComponent& atkState = ComponentArray<AttackStateComponent>::Get().GetComponent(entity);
      StateComponent& stateComp = ComponentArray<StateComponent>::Get().GetComponent(entity);

      if (atkState.lastFrame != animator.frame)
      {
        int frame = animator.frame;

        // update all in progress events now
        /*for (int i = 0; i < atkState.inProgressEvents.size(); i++)
        {
          AnimationEvent* evt = atkState.inProgressEvents[i];
          if (frame >= evt->GetEndFrame())
          {
            evt->EndEvent(entity);
            atkState.inProgressEvents.erase(atkState.inProgressEvents.begin() + i);
            i--;
          }
          else
          {
            evt->UpdateEvent(frame, entity, &transform, &stateComp);
          }
        }*/

        // Checks if an event should be trigger this frame of animation and calls its callback if so
        EventList& linkedEventList = *GAnimArchive.GetCollection(animator.animCollectionID).GetEventList(atkState.attackAnimation);
        std::vector<AnimationEvent>& potentialEvents = linkedEventList[frame];

        if (!potentialEvents.empty())
        {
          for (auto& evt : potentialEvents)
          {
            evt.TriggerEvent(entity, &transform, &stateComp);
            atkState.inProgressEventTypes.insert(evt.type);
          }
        }

        for (int f = 0; f <= frame; f++)
        {
          auto& frameEvtTriggers = linkedEventList[f];

          if (!frameEvtTriggers.empty())
          {
            for (auto& evt : frameEvtTriggers)
            {
              if (frame < evt.GetEndFrame() && frame > evt.GetStartFrame())
              {
                evt.UpdateEvent(frame, entity, &transform, &stateComp);
              }
              else if (frame == evt.GetEndFrame())
              {
                evt.EndEvent(entity);
              }
            }
          }
        }

        // update the last frame updated
        atkState.lastFrame = animator.frame;
      }
    }
  }
};

class AnimationSystem : public ISystem<Animator, RenderComponent<RenderType>, RenderProperties>
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
    for (const EntityID& entity : Registered)
    {
      PROFILE_FUNCTION();
      Animator& animator = ComponentArray<Animator>::Get().GetComponent(entity);
      RenderComponent<RenderType>& renderer = ComponentArray<RenderComponent<RenderType>>::Get().GetComponent(entity);
      RenderProperties& properties = ComponentArray<RenderProperties>::Get().GetComponent(entity);

      // if playing, do advance time and update frame
      if (animator.playing)
      {
        // when the animation is complete, do the listener callback
        // do this on the following frame so that the last frame of animation can still render
        if (auto* listener = animator.GetListener())
        {
          if (!animator.looping && animator.frame == (GAnimArchive.GetAnimationData(animator.animCollectionID, animator.currentAnimationName)->GetFrameCount() - 1))
            listener->OnAnimationComplete(animator.currentAnimationName);
        }

        if (animator.accumulatedTime >= secPerFrame)
        {
          int framesToAdv = (int)std::floor(animator.accumulatedTime / secPerFrame);

          // get next frame off of the type of anim it is
          int totalAnimFrames = GAnimArchive.GetAnimationData(animator.animCollectionID, animator.currentAnimationName)->GetFrameCount();

          int nextFrame = animator.looping ? GetNextFrameLooping(framesToAdv, animator.frame, totalAnimFrames)
            : GetNextFrameOnce(framesToAdv, animator.frame, totalAnimFrames);
          
          // frame has advanced, so we update the rect on source to reflect that
          if (nextFrame != animator.frame)
          {
            animator.frame = nextFrame;
            int currFrame = animator.reverse ? (totalAnimFrames - 1) - nextFrame : nextFrame;

            Animation* animation = GAnimArchive.GetAnimationData(animator.animCollectionID, animator.currentAnimationName);
            renderer.SetRenderResource(animation->GetSheetTexture<RenderType>());
            renderer.sourceRect = animation->GetFrameSrcRect(currFrame);
            properties.offset = animation->GetAnchorForAnimFrame(currFrame).second;
          }

          // 
          animator.accumulatedTime -= (framesToAdv * secPerFrame);
        }
        // update accumulated time by dt and the animation speed modifier
        animator.accumulatedTime += (dt * animator.playSpeed);
      }
    }
  }
};
