#pragma once
#include "Systems/ISystem.h"
#include "Components/GameActor.h"
#include "Components/Input.h"

#include "Components/Transform.h"
#include "Components/Rigidbody.h"

class PlayerSideSystem : public ISystem<RectColliderD, StateComponent>
{
public:
  static void DoTick(float dt)
  {
    for (auto& tuple : Tuples)
    {
      RectColliderD* collider = std::get<RectColliderD*>(tuple.second);
      StateComponent* state = std::get<StateComponent*>(tuple.second);

      for (auto& other : Tuples)
      {
        if (other != tuple)
        {
          state->onLeftSide = collider->rect.GetCenter().x < std::get<RectColliderD*>(other.second)->rect.GetCenter().x;
        }
      }
    }
  }
};

class InputSystem : public ISystem<KeyboardInputHandler, StateComponent, GameActor, Rigidbody>
{
public:
  static void DoTick(float dt)
  {
    for(auto& tuple : Tuples)
    {
      KeyboardInputHandler* inputHandler = std::get<KeyboardInputHandler*>(tuple.second);
      GameActor* actor = std::get<GameActor*>(tuple.second);
      Rigidbody* rigidbody = std::get<Rigidbody*>(tuple.second);
      StateComponent* state = std::get<StateComponent*>(tuple.second);

      const InputBuffer& unitInputState = inputHandler->CollectInputState();

      state->collision = rigidbody->_lastCollisionSide;

      actor->EvaluateInputContext(unitInputState, state, dt);

      rigidbody->elasticCollisions = actor->GetActionState() == ActionState::HITSTUN;
    }
  }
};

class GamepadInputSystem : public ISystem<GamepadInputHandler, StateComponent, GameActor, Rigidbody>
{
public:
  static void DoTick(float dt)
  {
    for(auto& tuple : Tuples)
    {
      GamepadInputHandler* inputHandler = std::get<GamepadInputHandler*>(tuple.second);
      GameActor* actor = std::get<GameActor*>(tuple.second);
      Rigidbody* rigidbody = std::get<Rigidbody*>(tuple.second);
      StateComponent* state = std::get<StateComponent*>(tuple.second);

      const InputBuffer& unitInputState = inputHandler->CollectInputState();

      state->collision = rigidbody->_lastCollisionSide;

      actor->EvaluateInputContext(unitInputState, state, dt);

      rigidbody->elasticCollisions = actor->GetActionState() == ActionState::HITSTUN;
    }
  }
};
