#pragma once
#include "Core/ECS/ISystem.h"
#include "Components/Input.h"
#include "Components/Actors/GameActor.h"

#include "Components/Transform.h"
#include "Components/Rigidbody.h"

class PlayerSideSystem : public ISystem<Transform, StateComponent, TeamComponent, GameActor, Actor>
{
public:
  static void DoTick(float dt)
  {
    for (const EntityID& entity : Registered)
    {
      Transform& transform = ComponentArray<Transform>::Get().GetComponent(entity);
      StateComponent& state = ComponentArray<StateComponent>::Get().GetComponent(entity);
      TeamComponent& teamComp = ComponentArray<TeamComponent>::Get().GetComponent(entity);
      GameActor& actor = ComponentArray<GameActor>::Get().GetComponent(entity);

      for (const EntityID& other : Registered)
      {
        if (other != entity)
        {
          Transform& otherTranform = ComponentArray<Transform>::Get().GetComponent(other);
          TeamComponent& otherEntityTeam = ComponentArray<TeamComponent>::Get().GetComponent(other);
          StateComponent& otherState = ComponentArray<StateComponent>::Get().GetComponent(other);

          if (otherState.onNewState)
          {
            if (otherState.actionState == ActionState::HITSTUN)
              state.comboCounter++;
            else
              state.comboCounter = 0;
          }

          bool lastSide = state.onLeftSide;
          if (teamComp.team != otherEntityTeam.team && otherEntityTeam.playerEntity)
          {
            state.onLeftSide = transform.position.x < otherTranform.position.x;
            if (state.onLeftSide != lastSide)
              actor.newInputs = true;
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
    for(const EntityID& entity : Registered)
    {
      GameInputComponent& inputHandler = ComponentArray<GameInputComponent>::Get().GetComponent(entity);
      GameActor& actor = ComponentArray<GameActor>::Get().GetComponent(entity);
      Rigidbody& rigidbody = ComponentArray<Rigidbody>::Get().GetComponent(entity);
      StateComponent& state = ComponentArray<StateComponent>::Get().GetComponent(entity);

      //const InputBuffer& unitInputState = inputHandler->QueryInput();
      // reset on new state
      state.onNewState = false;
      state.collision = rigidbody.lastCollisionSide;
      actor.TransferInputData(inputHandler.QueryInput(), &state);
      

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
