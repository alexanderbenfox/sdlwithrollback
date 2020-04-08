#pragma once
#include "Systems/ISystem.h"
#include "Components/Animator.h"
#include "Components/AttackStateComponent.h"

//!
//const float secPerFrame = 1.0f / animation_fps;
const float secPerFrame = 1.0f / 60.0f;

class AttackAnimationSystem : public ISystem<AttackStateComponent, AnimationRenderer, Transform>
{
public:
  static void DoTick(float dt)
  {
    for(auto tuple : Tuples)
    {
      AnimationRenderer* renderer = std::get<AnimationRenderer*>(tuple.second);
      Transform* transform = std::get<Transform*>(tuple.second);
      AttackStateComponent* atkState = std::get<AttackStateComponent*>(tuple.second);

      if (atkState->lastFrame != renderer->GetCurrentFrame())
      {
        atkState->CheckEvents(renderer->GetCurrentFrame(),
          (double)transform->position.x - (double)renderer->GetDisplayOffset().x * transform->scale.x, (double)transform->position.y - (double)renderer->GetDisplayOffset().y * transform->scale.y,
          transform);
        atkState->lastFrame = renderer->GetCurrentFrame();
      }
    }
  }
};

class DrawSystem : public ISystem<Transform, GraphicRenderer, RenderProperties>
{
  static void PostUpdate()
  {
    for (auto tuple : Tuples)
    {
      GraphicRenderer* renderer = std::get<GraphicRenderer*>(tuple.second);
      Transform* transform = std::get<Transform*>(tuple.second);
      RenderProperties* properties = std::get<RenderProperties*>(tuple.second);

      // get a display op to set draw parameters
      auto displayOp = ResourceManager::Get().GetAvailableOp();

      displayOp->_textureRect = renderer->sourceRect;
      displayOp->_textureResource = renderer->GetRenderResource();

      displayOp->_displayRect = OpSysConv::CreateSDLRect(
        static_cast<int>(std::floor(transform->position.x - properties->offset.x)),
        static_cast<int>(std::floor(transform->position.y - properties->offset.y)),
        (int)(static_cast<float>(renderer->sourceRect.w) * transform->scale.x),
        (int)(static_cast<float>(renderer->sourceRect.h) * transform->scale.y));

      // set properties
      displayOp->_flip = properties->GetFlip() ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
      // set display color directly
      displayOp->_displayColor = properties->GetDisplayColor();

      displayOp->valid = true;      
    }
  }
};

class AnimationSystem : public ISystem<Animator, GraphicRenderer>
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
      GraphicRenderer* renderer = std::get<GraphicRenderer*>(tuple.second);

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

        animator->accumulatedTime += dt;
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
      }
    }
  }
};
