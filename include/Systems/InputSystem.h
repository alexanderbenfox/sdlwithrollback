#pragma once
#include "Systems/ISystem.h"
#include "Components/GameActor.h"
#include "Components/Input.h"

#include "Components/Transform.h"
#include "Components/Rigidbody.h"

class PlayerSideSystem : public ISystem<Transform, StateComponent>
{
public:
  static void DoTick(float dt)
  {
    for (auto& tuple : Tuples)
    {
      Transform* transform = std::get<Transform*>(tuple.second);
      StateComponent* state = std::get<StateComponent*>(tuple.second);

      for (auto& other : Tuples)
      {
        if (other != tuple)
        {
          Transform* otherTranform = std::get<Transform*>(other.second);
          state->onLeftSide = transform->position.x < otherTranform->position.x;
        }
      }
    }
  }
};

class InputSystem : public ISystem<GameInputComponent, StateComponent, GameActor, Rigidbody>
{
public:
  static void DoTick(float dt)
  {
    for(auto& tuple : Tuples)
    {
      GameInputComponent* inputHandler = std::get<GameInputComponent*>(tuple.second);
      GameActor* actor = std::get<GameActor*>(tuple.second);
      Rigidbody* rigidbody = std::get<Rigidbody*>(tuple.second);
      StateComponent* state = std::get<StateComponent*>(tuple.second);

      const InputBuffer& unitInputState = inputHandler->QueryInput();

      state->collision = rigidbody->_lastCollisionSide;

      state->onNewState = false;

      // if evaluation leads to changing state, do on state change code
      if (state->onNewState = actor->EvaluateInputContext(unitInputState, state))
      {
        state->actionState = actor->GetActionState();
        state->stanceState = actor->GetStanceState();
        rigidbody->elasticCollisions = actor->GetActionState() == ActionState::HITSTUN;
      }
    }
  }
};
