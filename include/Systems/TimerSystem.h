#pragma once
#include "Systems/ISystem.h"
#include "Systems/AnimationSystem.h"
#include "Components/GameActor.h"

class TimerSystem : public ISystem<GameActor>
{
public:
  static void DoTick(float dt)
  {
    for(auto& tuple : Tuples)
    {
      std::vector<std::shared_ptr<TimerComponent>>& _timings = std::get<GameActor*>(tuple.second)->timings;
      std::vector<int> markedForDelete;
      for (int i = 0; i < _timings.size(); i++)
      {
        std::shared_ptr<TimerComponent> timer = _timings[i];
        if (timer->playTime >= secPerFrame)
        {
          int framesToAdv = (int)std::floor(timer->playTime / secPerFrame);

          if ((timer->currFrame + framesToAdv) >= timer->TotalFrames())
          {
            if(!timer->Cancelled())
              timer->OnComplete();
            markedForDelete.push_back(i);
          }
          else
          {
            timer->currFrame += framesToAdv;
          }

          timer->playTime -= (framesToAdv * secPerFrame);
        }
        // if playing, do advance time and update frame
        timer->playTime += dt;
      }

      int offset = 0;
      for (int& index : markedForDelete)
      {
        _timings.erase(_timings.begin() + (index + offset));
        offset--;
      }
    }
  }
};