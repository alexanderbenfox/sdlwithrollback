#pragma once
#include "Systems/ISystem.h"
#include "Components/GameActor.h"
#include "Components/Input.h"

#include "Components/Transform.h"
#include "Components/Rigidbody.h"

class InputSystem : public ISystem<KeyboardInputHandler, GameActor, Rigidbody, Transform, RectColliderD>
{
public:
  static void DoTick(float dt)
  {
    for(auto& tuple : Tuples)
    {
      KeyboardInputHandler* inputHandler = std::get<KeyboardInputHandler*>(tuple.second);
      GameActor* actor = std::get<GameActor*>(tuple.second);
      Rigidbody* rigidbody = std::get<Rigidbody*>(tuple.second);
      Transform* transform = std::get<Transform*>(tuple.second);
      RectColliderD* collider = std::get<RectColliderD*>(tuple.second);

      InputState unitInputState = inputHandler->CollectInputState();

      GameContext context;
      context.collision = rigidbody->_lastCollisionSide;

      for(auto& other : Tuples)
      {
        if(other != tuple)
        {
          Transform* otherTransform = std::get<Transform*>(other.second);
          RectColliderD* otherCollider = std::get<RectColliderD*>(other.second);

          int thisCenterX = transform->position.x + collider->rect.Width() / 2;
          int otherCenterX = otherTransform->position.x + otherCollider->rect.Width() / 2;

          context.onLeftSide = thisCenterX < otherCenterX;
        }
      }

      actor->EvaluateInputContext(unitInputState, context);
    }
  }
};