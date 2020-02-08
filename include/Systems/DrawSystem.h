#pragma once
#include "Systems/ISystem.h"
#include "Components/Animator.h"

class DrawSystem : public ISystem<Animator, Transform>
{
public:
  static void DoTick(float dt)
  {
    for (auto tuple : Tuples)
    {
      Animator* sprite = std::get<Animator*>(tuple.second);
      sprite->Advance(dt);
    }
  }

  static void PostUpdate()
  {
    for (auto tuple : Tuples)
    {
      // get a display op to set draw parameters
      auto displayOp = ResourceManager::Get().GetAvailableOp();

      Animator* sprite = std::get<Animator*>(tuple.second);
      Transform* transform = std::get<Transform*>(tuple.second);

      sprite->GetDisplayable()->SetOp(*transform, sprite->GetSourceRect(), sprite->GetDisplayOffset(), sprite->GetFlip(), displayOp);
    }
  }
};
