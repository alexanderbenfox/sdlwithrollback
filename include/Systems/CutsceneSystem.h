#pragma once
#include "Systems/ISystem.h"
#include "Components/Animator.h"

class CutsceneSystem : public ISystem<CutsceneActor, Animator, RenderComponent<RenderType>, RenderProperties>
{
public:
  static void DoTick(float dt)
  {
    bool scenePlaying = false;
    for(auto tuple : Tuples)
    {
      CutsceneActor* actor = std::get<CutsceneActor*>(tuple.second);
      Animator* animator = std::get<Animator*>(tuple.second);
      RenderComponent<RenderType>* renderer = std::get<RenderComponent<RenderType>*>(tuple.second);
      RenderProperties* properties = std::get<RenderProperties*>(tuple.second);

      CutsceneAction* action = nullptr;
      //check if actor has started yet
      if(!actor->started)
      {
        action = actor->ActionListPop();
        if(action)
          action->Begin(animator, renderer, properties);
      }
      else
      {
        action = actor->currentAction;
      }
      if(action)
      {
        // at least one has a current action, so scene is playing
        scenePlaying = true;

        if(action->isWaiting)
        {
          for(auto other : Tuples)
          {
            if(other != tuple)
            {
              if(action->CheckEndConditions(std::get<CutsceneActor*>(other.second)))
              {
                action->OnComplete();
                action = actor->ActionListPop();
                if(action)
                  action->Begin(animator, renderer, properties);
              }
            }
          }
        }
        else
        {
          if(action->CheckEndConditions())
          {
            action->OnComplete();
            action = actor->ActionListPop();
            if(action)
              action->Begin(animator, renderer, properties);
          }
        }
      }
    }

    // when scene is finished move to the results screen
    if(!scenePlaying)
    {
      GameManager::Get().RequestSceneChange(SceneType::RESULTS);
    }
  }
};

//! prevents movement on ground
class CutsceneMovementSystem : public ISystem<Rigidbody>
{
public:
  static void DoTick(float dt)
  {
    for(auto tuple : Tuples)
    {
      Rigidbody* rb = std::get<Rigidbody*>(tuple.second);
      if(rb->_lastCollisionSide == CollisionSide::DOWN)
      {
        rb->_vel = Vector2<float>::Zero;
      }
    }
  }
};
