#pragma once
#include "Systems/ISystem.h"
#include "Components/TimerContainer.h"

class TimerSystem : public ISystem<TimerContainer>
{
public:
  static void DoTick(float dt)
  {
    for(const EntityID& entity : Registered)
    {
      std::vector<std::shared_ptr<ActionTimer>>& _timings = ComponentArray<TimerContainer>::Get().GetComponent(entity).timings;
      std::vector<int> markedForDelete;
      for (int i = 0; i < _timings.size(); i++)
      {
        std::shared_ptr<ActionTimer> timer = _timings[i];

        // always update first
        timer->Update();

        if (timer->playTime >= secPerFrame)
        {
          int framesToAdv = (int)std::floor(timer->playTime / secPerFrame);

          if ((timer->currFrame + framesToAdv) >= timer->Duration())
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