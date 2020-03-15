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
            renderer->GetCurrentAnimation().CheckEvents(nextFrame,
              (double)transform->position.x - (double)renderer->GetDisplayOffset().x * transform->scale.x, (double)transform->position.y - (double)renderer->GetDisplayOffset().y * transform->scale.y,
              transform);
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


class TimerSystem : public ISystem<GameActor>
{
public:
  static void DoTick(float dt)
  {
    for (auto tuple : Tuples)
    {
      GameActor* actor = std::get<GameActor*>(tuple.second);
      std::vector<int> markedForDelete;
      for (int i = 0; i < actor->timings.size(); i++)
      {
        TimerComponent& timer = actor->timings[i];
        // if playing, do advance time and update frame
        timer.playTime += dt;
        if (timer.playTime >= secPerFrame)
        {
          int framesToAdv = (int)std::floor(timer.playTime / secPerFrame);

          if ((timer.currFrame + framesToAdv) >= timer.TotalFrames())
          {
            timer.OnComplete();
            markedForDelete.push_back(i);
          }
          else
          {
            timer.currFrame += framesToAdv;
          }

          timer.playTime -= (framesToAdv * secPerFrame);
        }
      }

      int offset = 0;
      for (int& index : markedForDelete)
      {
        actor->timings.erase(actor->timings.begin() + (index + offset));
        offset--;
      }

    }
  }
};