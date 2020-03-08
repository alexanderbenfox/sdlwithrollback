#pragma once
#include "Systems/ISystem.h"
#include "Components/Animator.h"

class DrawSystem : public ISystem<AnimationRenderer, Transform>
{
public:
  static void DoTick(float dt)
  {
    for (auto tuple : Tuples)
    {
      AnimationRenderer* renderer = std::get<AnimationRenderer*>(tuple.second);
      renderer->Advance(dt);
    }

    struct Routine
    {
      float _accumulatedTime;
      float _secPerFrame;
      int _frame;
      int _endFrame;
      std::function<void()> _callback;
      virtual int GetNextFrame(int frameToAdv) = 0;

      void Advance(float dt)
      {
        _accumulatedTime += dt;
        if (_accumulatedTime >= _secPerFrame)
        {
          int framesToAdv = (int)std::floor(_accumulatedTime / _secPerFrame);

          // get next frame off of the type of anim it is
          _frame = GetNextFrame(framesToAdv);

          // 
          _accumulatedTime -= (framesToAdv * _secPerFrame);

          // when the animation is complete, do the listener callback
          if(_frame == _endFrame)
            _callback();
        }
      }
    };
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
