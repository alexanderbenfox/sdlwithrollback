#pragma once
#include "Systems/ISystem.h"
#include "Components/Animator.h"
#include "Components/AttackStateComponent.h"
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
        AnimationEvent* potentialEvent = atkState->GetEventStartsThisFrame(frame);
        if (potentialEvent)
        {
          atkState->inProgressEvents.push_back(potentialEvent);
          potentialEvent->TriggerEvent(transform, stateComp);
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
        if (animator->GetListener())
        {
          if (!animator->looping && animator->frame == (animator->GetCurrentAnimation().GetFrameCount() - 1))
            animator->GetListener()->OnAnimationComplete(animator->currentAnimationName);
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
            renderer->sourceRect = animator->GetCurrentAnimation().GetFrameSrcRect(animator->frame);
          }

          // 
          animator->accumulatedTime -= (framesToAdv * secPerFrame);
        }
        animator->accumulatedTime += dt;
      }
    }
  }
};

class DrawSystem : public ISystem<Transform, RenderComponent<RenderType>, RenderProperties>
{
public:
  static void PostUpdate()
  {
    for (auto tuple : Tuples)
    {
      RenderComponent<RenderType>* renderer = std::get<RenderComponent<RenderType>*>(tuple.second);
      Transform* transform = std::get<Transform*>(tuple.second);
      RenderProperties* properties = std::get<RenderProperties*>(tuple.second);

      // if the render resource hasn't been assigned yet, hold off
      if(!renderer->GetRenderResource()) continue;

      // get a display op to set draw parameters
      auto displayOp = GRenderer.GetAvailableOp();

      displayOp->_textureRect = renderer->sourceRect;
      displayOp->_textureResource = renderer->GetRenderResource();

      Vector2<int> renderOffset = properties->Offset();

      displayOp->_displayRect = OpSysConv::CreateSDLRect(
        static_cast<int>(std::floor(transform->position.x + renderOffset.x * transform->scale.x)),
        static_cast<int>(std::floor(transform->position.y + renderOffset.y * transform->scale.y)),
        (int)(static_cast<float>(renderer->sourceRect.w) * transform->scale.x),
        (int)(static_cast<float>(renderer->sourceRect.h) * transform->scale.y));

      // set properties
      displayOp->_flip = properties->horizontalFlip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
      // set display color directly
      displayOp->_displayColor = properties->GetDisplayColor();

      displayOp->valid = true;      
    }
  }
};

class GLDrawSystem : public ISystem<Transform, TextRenderer, RenderProperties>
{
public:
  static void PostUpdate()
  {
    for (auto tuple : Tuples)
    {
      TextRenderer* renderer = std::get<TextRenderer*>(tuple.second);
      Transform* transform = std::get<Transform*>(tuple.second);
      RenderProperties* properties = std::get<RenderProperties*>(tuple.second);

      ResourceManager::Get().AppendDraw(renderer->GetRenderOps());
    }
  }
};

