#pragma once
#include "Systems/ISystem.h"
#include "Components/Input.h"
#include "Components/Actors/GameActor.h"

#include "Components/Transform.h"
#include "Components/Rigidbody.h"

class PlayerSideSystem : public ISystem<Transform, StateComponent, TeamComponent, GameActor, Actor>
{
public:
  static void DoTick(float dt)
  {
    for (auto& tuple : Tuples)
    {
      Transform* transform = std::get<Transform*>(tuple.second);
      StateComponent* state = std::get<StateComponent*>(tuple.second);
      TeamComponent* teamComp = std::get<TeamComponent*>(tuple.second);
      GameActor* actor = std::get<GameActor*>(tuple.second);

      for (auto& other : Tuples)
      {
        if (other != tuple)
        {
          Transform* otherTranform = std::get<Transform*>(other.second);
          TeamComponent* otherEntityTeam = std::get<TeamComponent*>(other.second);
          StateComponent* otherState = std::get<StateComponent*>(other.second);

          if (otherState->onNewState)
          {
            if (otherState->actionState == ActionState::HITSTUN)
              state->comboCounter++;
            else
              state->comboCounter = 0;

            otherState->onNewState = false;
          }


          bool lastSide = state->onLeftSide;
          if (teamComp->team != otherEntityTeam->team && otherEntityTeam->playerEntity)
          {
            state->onLeftSide = transform->position.x < otherTranform->position.x;
            if (state->onLeftSide != lastSide)
              actor->newInputs = true;
          }
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

      //const InputBuffer& unitInputState = inputHandler->QueryInput();

      state->collision = rigidbody->_lastCollisionSide;
      actor->TransferInputData(inputHandler->QueryInput(), state);
      

      /*state->onNewState = false;

      // if evaluation leads to changing state, do on state change code
      if ((state->onNewState = actor->EvaluateInputContext(unitInputState, state)))
      {
        state->actionState = actor->GetActionState();
        state->stanceState = actor->GetStanceState();
        //rigidbody->elasticCollisions = actor->GetActionState() == ActionState::HITSTUN;
      }*/
    }
  }
};
