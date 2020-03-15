#pragma once
#include "Systems/ISystem.h"
#include "Components/Hitbox.h"
#include "Components/GameActor.h"

//!
//const float secPerFrame = 1.0f / animation_fps;

class HitSystem : public ISystem<Hurtbox, GameActor>
{
public:
  static void DoTick(float dt)
  {
    for (auto tuple : Tuples)
    {
      Hurtbox* hurtbox = std::get<Hurtbox*>(tuple.second);
      GameActor* hurtboxController = std::get<GameActor*>(tuple.second);

      for (auto hitbox : ComponentManager<Hitbox>::Get().All())
      {
        if (!hitbox->ShareOwner(hurtbox) && hitbox->rect.Collides(hurtbox->rect))
        {
          // change the state variable that will be evaluated on the processing of inputs. probably a better way to do this...
          hurtboxController->mergeContext.hitThisFrame = true;
          double amtLeft = hurtbox->rect.GetCenter().x - hitbox->rect.Beg().x;
          double amtRight = hitbox->rect.End().x - hurtbox->rect.GetCenter().x;
          hurtboxController->mergeContext.hitOnLeftSide =  amtLeft > amtRight;
          hurtboxController->mergeContext.frameData = hitbox->frameData;
          if(amtLeft < amtRight)
            hurtboxController->mergeContext.frameData.knockback.x = -hitbox->frameData.knockback.x;
        }
      }
    }
  }
};
