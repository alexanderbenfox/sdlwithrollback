#include "Systems/ActionSystems/ActionHandleInputSystem.h"

#include "Core/Prefab/ActionFactory.h"
#include "GameManagement.h"

#include "Components/StateComponents/AttackStateComponent.h"

#include "StateMachine/ActionUtil.h"

//______________________________________________________________________________
void TimedActionSystem::DoTick(float dt)
{
  for (auto tuple : Tuples)
  {
    TimedActionComponent* timer = std::get<TimedActionComponent*>(tuple.second);
    GameActor* actor = std::get<GameActor*>(tuple.second);

    // if playing, do advance time and update frame
    timer->playTime += dt;

    int framesToAdv = (int)std::floor(timer->playTime / secPerFrame);
    timer->currFrame += framesToAdv;

    // reset the real-time timer
    timer->playTime -= (framesToAdv * secPerFrame);

    if (timer->currFrame >= (timer->totalFrames - 1))
    {
      actor->actionTimerComplete = true;
      // force check for new state here
      actor->forceNewInputOnNextFrame = true;
      // remove system flag at the end of the system call
      GameManager::Get().ScheduleTask([actor]() { actor->GetOwner()->RemoveComponent<TimedActionComponent>(); });
    }
  }
  GameManager::Get().RunScheduledTasks();
}


//______________________________________________________________________________
void HandleInputGrappledActionSystem::DoTick(float dt)
{
  for (auto tuple : Tuples)
  {
    ReceivedGrappleAction* timer = std::get<ReceivedGrappleAction*>(tuple.second);
    GameActor* actor = std::get<GameActor*>(tuple.second);
    StateComponent* state = std::get<StateComponent*>(tuple.second);

    // on timer complete, move to knockdown airborne state
    if (actor->actionTimerComplete)
    {
      ActionFactory::SetKnockdownAirborne(actor->GetOwner().get(), state);
      GameManager::Get().ScheduleTask([actor]()
        {
          actor->GetOwner()->RemoveComponent<ReceivedGrappleAction>();
          actor->GetOwner()->GetComponent<ReceivedDamageAction>()->fromGrapple = true;
        });
    }
  }

  ActionFactory::DisableActionListenerForEntities();
  GameManager::Get().RunScheduledTasks();
}

//______________________________________________________________________________
void HandleDashUpdateSystem::DoTick(float dt)
{
  for (auto tuple : Tuples)
  {
    DashingAction* action = std::get<DashingAction*>(tuple.second);
    Rigidbody* rb = std::get<Rigidbody*>(tuple.second);
    TimedActionComponent* timer = std::get<TimedActionComponent*>(tuple.second);

    float t = static_cast<float>(timer->currFrame);
    float totalT = static_cast<float>(timer->totalFrames);

    float f_t = Interpolation::Plateau::F(t, totalT, action->dashSpeed);
    rb->_vel = Vector2<float>(f_t, 0);
  }
}

//______________________________________________________________________________
void HandleInputJump::DoTick(float dt)
{
  for (auto tuple : Tuples)
  {
    Rigidbody* rb = std::get<Rigidbody*>(tuple.second);
    
    if (HasState(rb->_lastCollisionSide, CollisionSide::DOWN))
    {
      GameActor* actor = std::get<GameActor*>(tuple.second);
      auto entity = actor->GetOwner();

      ActionFactory::GoToNeutralAction(entity.get(), entity->GetComponent<StateComponent>().get());
      GameManager::Get().ScheduleTask([entity]()
      {
        entity->RemoveComponent<JumpingAction>();
      });
    }
  }
  GameManager::Get().RunScheduledTasks();
}

//______________________________________________________________________________
void HandleInputCrouch::DoTick(float dt)
{
  for (auto tuple : Tuples)
  {
    GameActor* actor = std::get<GameActor*>(tuple.second);

    if (!HasState(actor->LastButtons(), InputState::DOWN))
    {
      auto entity = actor->GetOwner();
      GameManager::Get().ScheduleTask([entity]()
      {
        entity->RemoveComponent<CrouchingAction>();
        entity->RemoveComponent<TransitionToCrouching>();
      });
    }
  }
  GameManager::Get().RunScheduledTasks();
}

//______________________________________________________________________________
void HandleInputGrappling::DoTick(float dt)
{
  for (auto tuple : Tuples)
  {
    StateComponent* state = std::get<StateComponent*>(tuple.second);
    GameActor* actor = std::get<GameActor*>(tuple.second);

    if (state->triedToThrowThisFrame && !state->throwSuccess)
    {
      auto entity = actor->GetOwner();
      GameManager::Get().ScheduleTask([entity, state]()
      {
        entity->AddComponent<EnactActionComponent>();
        // set up animation
        entity->AddComponent<AnimatedActionComponent>({ state->onLeftSide, false, true, 1.0f, "ThrowMiss" });
        entity->AddComponent<WaitForAnimationComplete>();
        
        entity->RemoveComponent<AttackActionComponent>();
        entity->RemoveComponent<AttackStateComponent>();
        entity->RemoveComponent<GrappleActionComponent>();

        // set empty component for follow up action
        entity->AddComponent<TransitionToNeutral>();

      });
      ActionFactory::SetEntityDecided(entity.get());
    }

    // check to see if the grappling action was cancelled by getting hit

  }
  ActionFactory::DisableActionListenerForEntities();
  GameManager::Get().RunScheduledTasks();
}

//______________________________________________________________________________
void CheckForMove::DoTick(float dt)
{
  for (auto tuple : Tuples)
  {
    Rigidbody* rigidbody = std::get<Rigidbody*>(tuple.second);
    GameActor* actor = std::get<GameActor*>(tuple.second);
    StateComponent* state = std::get<StateComponent*>(tuple.second);

    // N/A if not grounded
    if (!actor->newInputs || !HasState(rigidbody->_lastCollisionSide, CollisionSide::DOWN))
      continue;

    if (HasState(actor->LastButtons(), InputState::LEFT) || HasState(actor->LastButtons(), InputState::RIGHT))
    {
      Vector2<float> movementVector;
      if (HasState(actor->LastButtons(), InputState::LEFT))
        movementVector = Vector2<float>(-0.5f * ActionParams::baseWalkSpeed, 0.0f);
      else if (HasState(actor->LastButtons(), InputState::RIGHT))
        movementVector = Vector2<float>(0.5f * ActionParams::baseWalkSpeed, 0.0f);


      GameManager::Get().ScheduleTask([actor, state, movementVector]()
      {
        // Always reset action complete flag on new action
        actor->actionTimerComplete = false;

        actor->GetOwner()->AddComponent<EnactActionComponent>();

        // set animation based on direction
        std::string animation = state->onLeftSide ? (HasState(actor->LastButtons(), InputState::RIGHT) ? "WalkF" : "WalkB") : (HasState(actor->LastButtons(), InputState::LEFT) ? "WalkF" : "WalkB");
        actor->GetOwner()->AddComponent<AnimatedActionComponent>({ state->onLeftSide, true, false, 1.0f, animation });

        actor->GetOwner()->AddComponent<MovingActionComponent>();
        actor->GetOwner()->GetComponent<MovingActionComponent>()->velocity = movementVector;
        actor->GetOwner()->GetComponent<MovingActionComponent>()->horizontalMovementOnly = true;

        // add states for potential outside influence
        actor->GetOwner()->AddComponent<HittableState>();
        actor->GetOwner()->GetComponent<HittableState>()->canBlock = true;
        actor->GetOwner()->GetComponent<HittableState>()->inKnockdown = false;

        // enable all abilities in neutral
        ActionFactory::EnableAbility(actor->Owner());
      });

      ActionFactory::SetEntityDecided(actor->Owner());
    }
    else if (!HasState(actor->LastButtons(), InputState::DOWN))
    {
      ActionFactory::GoToNeutralAction(actor->GetOwner().get(), state);
    }
  }
  ActionFactory::DisableActionListenerForEntities();
  GameManager::Get().RunScheduledTasks();
}

//______________________________________________________________________________
void CheckForJump::DoTick(float dt)
{
  for (auto tuple : Tuples)
  {
    Rigidbody* rigidbody = std::get<Rigidbody*>(tuple.second);
    GameActor* actor = std::get<GameActor*>(tuple.second);
    StateComponent* state = std::get<StateComponent*>(tuple.second);

    // N/A if not grounded
    if (!HasState(rigidbody->_lastCollisionSide, CollisionSide::DOWN))
      continue;

    if (HasState(actor->LastButtons(), InputState::UP))
    {
      Vector2<float> movementVector;
      if (HasState(actor->LastButtons(), InputState::LEFT))
        movementVector = Vector2<float>(-0.5f * ActionParams::baseWalkSpeed, -UniversalPhysicsSettings::Get().JumpVelocity);
      else if (HasState(actor->LastButtons(), InputState::RIGHT))
        movementVector = Vector2<float>(0.5f * ActionParams::baseWalkSpeed, -UniversalPhysicsSettings::Get().JumpVelocity);
      else
        movementVector = Vector2<float>(0.0f, -UniversalPhysicsSettings::Get().JumpVelocity);

      GameManager::Get().ScheduleTask([actor, state, movementVector]()
      {
        actor->GetOwner()->AddComponent<EnactActionComponent>();

        actor->GetOwner()->AddComponent<MovingActionComponent>();
        actor->GetOwner()->GetComponent<MovingActionComponent>()->velocity = movementVector;
        actor->GetOwner()->GetComponent<MovingActionComponent>()->horizontalMovementOnly = false;

        actor->GetOwner()->AddComponent<AnimatedActionComponent>({ state->onLeftSide, false, false, 1.0f, "Jumping" });

        // transition when fully off the ground
        actor->GetOwner()->AddComponent<WaitingForJumpAirborne>();

        //
        actor->GetOwner()->RemoveComponent<AbleToJump>();
      });
      ActionFactory::SetEntityDecided(actor->Owner());
    }
  }
  ActionFactory::DisableActionListenerForEntities();
  GameManager::Get().RunScheduledTasks();
}

//______________________________________________________________________________
void CheckForFalling::DoTick(float dt)
{
  for (auto tuple : Tuples)
  {
    Rigidbody* rigidbody = std::get<Rigidbody*>(tuple.second);
    GameActor* actor = std::get<GameActor*>(tuple.second);
    StateComponent* state = std::get<StateComponent*>(tuple.second);

    if (!HasState(rigidbody->_lastCollisionSide, CollisionSide::DOWN))
    {
      ActionFactory::SetAerialState(actor->Owner());
      GameManager::Get().ScheduleTask([actor, state]()
      {
        actor->GetOwner()->AddComponent<AnimatedActionComponent>({ state->onLeftSide, false, true, 1.0f, "Falling" });

        // because we're falling, remove any movement component
        actor->GetOwner()->RemoveComponent<MovingActionComponent>();
      });
    }
  }
  GameManager::Get().RunScheduledTasks();
}

//______________________________________________________________________________
void CheckForBeginCrouching::DoTick(float dt)
{
  for (auto tuple : Tuples)
  {
    Rigidbody* rigidbody = std::get<Rigidbody*>(tuple.second);
    GameActor* actor = std::get<GameActor*>(tuple.second);
    StateComponent* state = std::get<StateComponent*>(tuple.second);

    // N/A if not grounded
    if (!HasState(rigidbody->_lastCollisionSide, CollisionSide::DOWN))
      continue;

    if (HasState(actor->LastButtons(), InputState::DOWN))
    {
      GameManager::Get().ScheduleTask([actor, state]()
      {
        actor->GetOwner()->AddComponent<EnactActionComponent>();

        actor->GetOwner()->AddComponent<MovingActionComponent>();
        actor->GetOwner()->GetComponent<MovingActionComponent>()->velocity = Vector2<float>::Zero;
        actor->GetOwner()->GetComponent<MovingActionComponent>()->horizontalMovementOnly = true;

        actor->GetOwner()->AddComponent<AnimatedActionComponent>({ state->onLeftSide, false, false, 1.0f, "Crouching" });
        actor->GetOwner()->AddComponent<WaitForAnimationComplete>();
        actor->GetOwner()->AddComponent<TransitionToCrouching>();
        actor->GetOwner()->AddComponent<CrouchingAction>();

        actor->GetOwner()->RemoveComponent<AbleToCrouch>();
      });
      //ActionFactory::SetEntityDecided(actor->Owner());
    }
  }
  //ActionFactory::DisableActionListenerForEntities();
  GameManager::Get().RunScheduledTasks();
}

//______________________________________________________________________________
void CheckHitThisFrameSystem::DoTick(float dt)
{
  for (auto tuple : Tuples)
  {
    StateComponent* state = std::get<StateComponent*>(tuple.second);
    HittableState* hittable = std::get<HittableState*>(tuple.second);
    GameActor* actor = std::get<GameActor*>(tuple.second);

    if (state->thrownThisFrame)
    {
      ActionFactory::SetGrappledAction(state->Owner(), state);
      continue;
    }
    else if (state->hitThisFrame)
    {
      // consume the hit this frame flag
      state->hitThisFrame = false;

      InputState const& buttons = actor->LastButtons();
      if (hittable->canBlock)
      {
        bool blockedRight = HasState(buttons, InputState::LEFT) && state->onLeftSide;
        bool blockedLeft = HasState(buttons, InputState::RIGHT) && !state->onLeftSide;
        if (blockedRight || blockedLeft)
        {
          ActionFactory::SetBlockStunAction(state->Owner(), state, HasState(buttons, InputState::DOWN));
          continue;
        }
      }

      // if already in knockdown or this attack puts player in knockdown
      if (hittable->inKnockdown || state->hitData.knockdown)
      {
        ActionFactory::SetKnockdownAirborne(state->Owner(), state);
        continue;
      }

      // default to hitstun state
      ActionFactory::SetHitStunAction(state->Owner(), state, HasState(buttons, InputState::DOWN));
    }
  }
  ActionFactory::DisableActionListenerForEntities();
  GameManager::Get().RunScheduledTasks();
}

//______________________________________________________________________________
void CheckSpecialAttackInputSystem::DoTick(float dt)
{
  for (auto tuple : Tuples)
  {
    StateComponent* state = std::get<StateComponent*>(tuple.second);
    GameActor* actor = std::get<GameActor*>(tuple.second);

    // only for ryu right now... probably need some kind of move mapping component
    if (HasState(actor->LastButtons(), InputState::BTN1) || HasState(actor->LastButtons(), InputState::BTN2) || HasState(actor->LastButtons(), InputState::BTN3))
    {
      bool fireball = (actor->LastSpecial() == SpecialInputState::QCF && state->onLeftSide) || (actor->LastSpecial() == SpecialInputState::QCB && !state->onLeftSide);
      bool donkeyKick = fireball && (HasState(actor->LastButtons(), InputState::BTN3));
      bool tatsu = (actor->LastSpecial() == SpecialInputState::QCF && !state->onLeftSide) || (actor->LastSpecial() == SpecialInputState::QCB && state->onLeftSide);
      bool dp = (actor->LastSpecial() == SpecialInputState::DPF && state->onLeftSide) || (actor->LastSpecial() == SpecialInputState::DPB && !state->onLeftSide);
      if (fireball || donkeyKick || tatsu || dp)
      {
        if (donkeyKick)
          ActionFactory::SetAttackAction(state->Owner(), state, "SpecialMove3", ActionState::HEAVY);
        else if (fireball)
          ActionFactory::SetAttackAction(state->Owner(), state, "SpecialMove1", ActionState::HEAVY);
        else if (tatsu)
          ActionFactory::SetAttackAction(state->Owner(), state, "SpecialMove4", ActionState::HEAVY);
        else if (dp)
          ActionFactory::SetAttackAction(state->Owner(), state, "SpecialMove2", ActionState::HEAVY);

        GameManager::Get().ScheduleTask([state]()
        {
          // add additional cancelables for special moves here
          state->Owner()->AddComponents<CancelOnDash, CancelOnJump>();

          // no cancel on hit ground for stuff like tatsu
          state->Owner()->RemoveComponents<CancelOnHitGround, CancelOnNormal, CancelOnSpecial>();
          state->Owner()->RemoveComponent<CrouchingAction>();
        });

      }
    }
  }
  ActionFactory::DisableActionListenerForEntities();
  GameManager::Get().RunScheduledTasks();
}

//______________________________________________________________________________
void CheckDashSystem::DoTick(float dt)
{
  for (auto tuple : Tuples)
  {
    StateComponent* state = std::get<StateComponent*>(tuple.second);
    GameActor* actor = std::get<GameActor*>(tuple.second);
    Animator* animator = std::get<Animator*>(tuple.second);
    Rigidbody* rigidbody = std::get<Rigidbody*>(tuple.second);

    if (!HasState(rigidbody->_lastCollisionSide, CollisionSide::DOWN))
      continue;

    const InputState& btns = actor->LastButtons();
    const SpecialInputState& sp = actor->LastSpecial();

    if (HasState(btns, InputState::LEFT))
    {
      if (HasState(btns, InputState::BTN4) || sp == SpecialInputState::LDash)
      {
        ActionFactory::SetDashAction(actor->Owner(), state, animator, !state->onLeftSide);
      }
    }
    else if (HasState(btns, InputState::RIGHT))
    {
      if (HasState(btns, InputState::BTN4) || sp == SpecialInputState::RDash)
      {
        ActionFactory::SetDashAction(actor->Owner(), state, animator, state->onLeftSide);
      }
    }
  }
  ActionFactory::DisableActionListenerForEntities();
  GameManager::Get().RunScheduledTasks();
}

//______________________________________________________________________________
void CheckAttackInputSystem::DoTick(float dt)
{
  for (auto tuple : Tuples)
  {
    StateComponent* state = std::get<StateComponent*>(tuple.second);
    GameActor* actor = std::get<GameActor*>(tuple.second);

    if (HasState(state->collision, CollisionSide::DOWN))
    {
      // first check throws
      if (HasState(actor->LastButtons(), InputState::BTN4))
      {
        bool backthrow = (state->onLeftSide && HasState(actor->LastButtons(), InputState::LEFT)) || (!state->onLeftSide && HasState(actor->LastButtons(), InputState::RIGHT));
        ActionFactory::SetAttackAction(state->Owner(), state, backthrow ? "BackThrow" : "ForwardThrow", ActionState::NONE);
        state->Owner()->AddComponent<GrappleActionComponent>();
        continue;
      }

      bool crouchPossible = HasState(actor->LastButtons(), InputState::DOWN);
      // then check attacks
      if (HasState(actor->LastButtons(), InputState::BTN1))
      {
        ActionFactory::SetAttackAction(state->Owner(), state, crouchPossible ? "CrouchingLight" : "StandingLight", ActionState::LIGHT);
      }
      else if (HasState(actor->LastButtons(), InputState::BTN2))
      {
        ActionFactory::SetAttackAction(state->Owner(), state, crouchPossible ? "CrouchingMedium" : "StandingMedium", ActionState::MEDIUM);
      }
      else if (HasState(actor->LastButtons(), InputState::BTN3))
      {
        ActionFactory::SetAttackAction(state->Owner(), state, crouchPossible ? "CrouchingHeavy" : "StandingHeavy", ActionState::HEAVY);
      }
    }
    else
    {
      if (HasState(actor->LastButtons(), InputState::BTN1))
      {
        ActionFactory::SetAttackAction(state->Owner(), state, "JumpingLight", ActionState::LIGHT);
      }
      else if (HasState(actor->LastButtons(), InputState::BTN2))
      {
        ActionFactory::SetAttackAction(state->Owner(), state, "JumpingMedium", ActionState::MEDIUM);
      }
      else if (HasState(actor->LastButtons(), InputState::BTN3))
      {
        ActionFactory::SetAttackAction(state->Owner(), state, "JumpingHeavy", ActionState::HEAVY);
      }
    }
  }
  ActionFactory::DisableActionListenerForEntities();
  GameManager::Get().RunScheduledTasks();
}

//______________________________________________________________________________
void CheckGrappleCancelOnHit::DoTick(float dt)
{
  for (auto t1 : MainSystem::Tuples)
  {
    StateComponent* grapplerState = std::get<StateComponent*>(t1.second);
    if (grapplerState->hitThisFrame)
    {
      for (auto t2 : SubSystem::Tuples)
      {
        Transform* transform = std::get<Transform*>(t2.second);
        StateComponent* state = std::get<StateComponent*>(t2.second);

        state->thrownThisFrame = false;
        GameManager::Get().ScheduleTask([transform]() { transform->RemoveComponent<ReceivedGrappleAction>(); });
      }
    }
  }
  GameManager::Get().RunScheduledTasks();
}

//______________________________________________________________________________
void ListenForAirborneSystem::DoTick(float dt)
{
  for (auto tuple : Tuples)
  {
    Rigidbody* rigidbody = std::get<Rigidbody*>(tuple.second);
    GameActor* actor = std::get<GameActor*>(tuple.second);
    if (!HasState(rigidbody->_lastCollisionSide, CollisionSide::DOWN))
    {
      ActionFactory::SetAerialState(actor->Owner());
    }
  }
  GameManager::Get().RunScheduledTasks();
}

//______________________________________________________________________________
void TransitionToNeutralSystem::DoTick(float dt)
{
  for (auto tuple : Tuples)
  {
    StateComponent* state = std::get<StateComponent*>(tuple.second);
    GameActor* actor = std::get<GameActor*>(tuple.second);
    Rigidbody* rigidbody = std::get<Rigidbody*>(tuple.second);

    if (actor->actionTimerComplete)
    {
      ActionFactory::GoToNeutralAction(state->Owner(), state);
      if (HasState(actor->LastButtons(), InputState::DOWN) && HasState(rigidbody->_lastCollisionSide, CollisionSide::DOWN))
      {
        ActionFactory::SetCrouchingState(state->Owner(), state);
      }
      else if (!HasState(rigidbody->_lastCollisionSide, CollisionSide::DOWN))
      {
        //ActionFactory::SetAerialState(state->Owner());
      }
    }
  }
  GameManager::Get().RunScheduledTasks();
}

//______________________________________________________________________________
void CheckKnockdownComplete::DoTick(float dt)
{
  for (auto tuple : Tuples)
  {
    StateComponent* state = std::get<StateComponent*>(tuple.second);
    GameActor* actor = std::get<GameActor*>(tuple.second);

    if (actor->actionTimerComplete)
    {
      ActionFactory::SetKnockdownGroundInvincible(state->Owner(), state);
    }
  }
  ActionFactory::DisableActionListenerForEntities();
  GameManager::Get().RunScheduledTasks();
}

//______________________________________________________________________________
void CheckKnockdownOTG::DoTick(float dt)
{
  for (auto tuple : Tuples)
  {
    StateComponent* state = std::get<StateComponent*>(tuple.second);
    Rigidbody* rigidbody = std::get<Rigidbody*>(tuple.second);

    if (HasState(rigidbody->_lastCollisionSide, CollisionSide::DOWN))
    {
      ActionFactory::SetKnockdownGroundOTG(state->Owner(), state);
    }
  }
  ActionFactory::DisableActionListenerForEntities();
  GameManager::Get().RunScheduledTasks();
}

//______________________________________________________________________________
void CheckCrouchingFollowUp::DoTick(float dt)
{
  for (auto tuple : Tuples)
  {
    StateComponent* state = std::get<StateComponent*>(tuple.second);
    GameActor* actor = std::get<GameActor*>(tuple.second);

    if (actor->actionTimerComplete)
    {
      ActionFactory::SetCrouchingState(actor->Owner(), state);
    }
  }
  GameManager::Get().RunScheduledTasks();
}

//______________________________________________________________________________
void HitGroundCancelActionSystem::DoTick(float dt)
{
  for (auto tuple : Tuples)
  {
    Rigidbody* rb = std::get<Rigidbody*>(tuple.second);
    GameActor* actor = std::get<GameActor*>(tuple.second);
    if (HasState(rb->_lastCollisionSide, CollisionSide::DOWN))
    {
      actor->actionTimerComplete = true;
    }
  }
}

//______________________________________________________________________________
void SpecialMoveCancelActionSystem::DoTick(float dt)
{
  for (auto tuple : Tuples)
  {
    GameActor* actor = std::get<GameActor*>(tuple.second);
    StateComponent* state = std::get<StateComponent*>(tuple.second);

    // can only be cancelled on hit for now (replace with HittingComponent maybe?)
    if(state->hitting && ActionFactory::ActorDidSpecialInputRyu(actor, state))
    {
      GameManager::Get().ScheduleTask([actor]()
      {
        actor->GetOwner()->AddComponent<AbleToSpecialAttackState>();
        actor->GetOwner()->RemoveComponent<CancelOnSpecial>();
      });
    }
  }
  GameManager::Get().RunScheduledTasks();
}

//______________________________________________________________________________
void TargetComboCancelActionSystem::DoTick(float dt)
{
  for (auto tuple : Tuples)
  {
    GameActor* actor = std::get<GameActor*>(tuple.second);
    StateComponent* state = std::get<StateComponent*>(tuple.second);
    HasTargetCombo* comboableMap = std::get<HasTargetCombo*>(tuple.second);

    // can only be cancelled on hit for now (replace with HittingComponent maybe?)
    if (state->hitting)
    {
      // if has a combo for this state and player is pressing that corresponding input
      if (comboableMap->links.find(state->actionState) != comboableMap->links.end())
      {
        if (comboableMap->links[state->actionState] == actor->LastButtons())
        {
          GameManager::Get().ScheduleTask([actor]()
          {
            actor->GetOwner()->AddComponent<AbleToAttackState>();
            actor->GetOwner()->RemoveComponent<CancelOnNormal>();
          });
        }
      }
    }
  }
  GameManager::Get().RunScheduledTasks();
}
