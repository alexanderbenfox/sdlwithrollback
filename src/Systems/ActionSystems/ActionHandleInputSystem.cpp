#include "Systems/ActionSystems/ActionHandleInputSystem.h"

#include "Core/Prefab/ActionFactory.h"
#include "Managers/GameManagement.h"

#include "Components/StateComponents/AttackStateComponent.h"

//______________________________________________________________________________
void TimedActionSystem::DoTick(float dt)
{
  DeferScopeGuard guard;
  for (const EntityID& entity : Registered)
  {
    TimedActionComponent& timer = ComponentArray<TimedActionComponent>::Get().GetComponent(entity);
    GameActor& actor = ComponentArray<GameActor>::Get().GetComponent(entity);

    // if playing, do advance time and update frame
    timer.playTime += dt;

    int framesToAdv = (int)std::floor(timer.playTime / secPerFrame);
    timer.currFrame += framesToAdv;

    // reset the real-time timer
    timer.playTime -= (framesToAdv * secPerFrame);

    if (timer.currFrame >= (timer.totalFrames - 1))
    {
      actor.actionTimerComplete = true;
      // force check for new state here
      actor.forceNewInputOnNextFrame = true;
      // remove system flag at the end of the system call
      
      defer(entity, GameManager::Get().GetEntityByID(entity)->RemoveComponent<TimedActionComponent>());
      //guard.deferred.emplace(guard.deferred.begin(), [&](){ GameManager::Get().GetEntityByID(entity)->RemoveComponent<TimedActionComponent>(); });
    }
  }
}

//______________________________________________________________________________
void HandleInputGrappledActionSystem::DoTick(float dt)
{
  DeferScopeGuard guard;
  for (const EntityID& entity : Registered)
  {
    ReceivedGrappleAction& timer = ComponentArray<ReceivedGrappleAction>::Get().GetComponent(entity);
    GameActor& actor = ComponentArray<GameActor>::Get().GetComponent(entity);
    StateComponent& state = ComponentArray<StateComponent>::Get().GetComponent(entity);

    // on timer complete, move to knockdown airborne state
    if (actor.actionTimerComplete)
    {
      defer((entity, &state),
      {
        ActionFactory::SetKnockdownAirborne(entity, &state);
        GameManager::Get().GetEntityByID(entity)->RemoveComponent<ReceivedGrappleAction>();
        GameManager::Get().GetEntityByID(entity)->GetComponent<ReceivedDamageAction>()->fromGrapple = true;
      });
    }
  }
}

//______________________________________________________________________________
void HandleDashUpdateSystem::DoTick(float dt)
{
  for (const EntityID& entity : Registered)
  {
    DashingAction& action = ComponentArray<DashingAction>::Get().GetComponent(entity);
    Rigidbody& rb = ComponentArray<Rigidbody>::Get().GetComponent(entity);
    TimedActionComponent& timer = ComponentArray<TimedActionComponent>::Get().GetComponent(entity);

    float t = static_cast<float>(timer.currFrame);
    float totalT = static_cast<float>(timer.totalFrames);

    float f_t = Interpolation::Plateau::F(t, totalT, action.dashSpeed);
    rb._vel = Vector2<float>(f_t, 0);
  }
}

//______________________________________________________________________________
void HandleInputJump::DoTick(float dt)
{
  DeferScopeGuard guard;
  for (const EntityID& entity : Registered)
  {
    Rigidbody& rb = ComponentArray<Rigidbody>::Get().GetComponent(entity);
    StateComponent& state = ComponentArray<StateComponent>::Get().GetComponent(entity);

    if (HasState(rb._lastCollisionSide, CollisionSide::DOWN))
    {
      defer((entity, &state),
        ActionFactory::GoToNeutralAction(entity, &state);
        GameManager::Get().GetEntityByID(entity)->RemoveComponent<JumpingAction>();
      );
    }
  }
}

//______________________________________________________________________________
void HandleInputCrouch::DoTick(float dt)
{
  DeferScopeGuard guard;
  for (const EntityID& entity : Registered)
  {
    GameActor& actor = ComponentArray<GameActor>::Get().GetComponent(entity);

    if (!HasState(actor.LastButtons(), InputState::DOWN))
    {
      defer(entity,
        GameManager::Get().GetEntityByID(entity)->RemoveComponent<CrouchingAction>();
        GameManager::Get().GetEntityByID(entity)->RemoveComponent<TransitionToCrouching>();

        if (GameManager::Get().GetEntityByID(entity)->GetComponent<AbleToAttackState>())
        {
          GameManager::Get().GetEntityByID(entity)->AddComponent<AbleToWalkLeft>();
          GameManager::Get().GetEntityByID(entity)->AddComponent<AbleToWalkRight>();
        });
    }
  }
}

//______________________________________________________________________________
void HandleInputGrappling::DoTick(float dt)
{
  DeferScopeGuard guard;
  for (const EntityID& entity : Registered)
  {
    StateComponent& state = ComponentArray<StateComponent>::Get().GetComponent(entity);
    GameActor& actor = ComponentArray<GameActor>::Get().GetComponent(entity);

    if (state.triedToThrowThisFrame && !state.throwSuccess)
    {
      defer((entity, state), {
        GameManager::Get().GetEntityByID(entity)->AddComponent<EnactActionComponent>();
        // set up animation
        GameManager::Get().GetEntityByID(entity)->AddComponent<AnimatedActionComponent>({ state.onLeftSide, false, true, 1.0f, "ThrowMiss" });
        GameManager::Get().GetEntityByID(entity)->AddComponent<WaitForAnimationComplete>();
        
        GameManager::Get().GetEntityByID(entity)->RemoveComponent<AttackActionComponent>();
        GameManager::Get().GetEntityByID(entity)->RemoveComponent<AttackStateComponent>();
        GameManager::Get().GetEntityByID(entity)->RemoveComponent<GrappleActionComponent>();

          // set empty component for follow up action
        GameManager::Get().GetEntityByID(entity)->AddComponent<TransitionToNeutral>();
        GameManager::Get().GetEntityByID(entity)->RemoveComponent<InputListenerComponent>();
      });
    }
  }
}

//______________________________________________________________________________
void CheckForReturnToNeutral::DoTick(float dt)
{
  DeferScopeGuard guard;
  for (const EntityID& entity : Registered)
  {
    Rigidbody& rigidbody = ComponentArray<Rigidbody>::Get().GetComponent(entity);
    GameActor& actor = ComponentArray<GameActor>::Get().GetComponent(entity);
    StateComponent& state = ComponentArray<StateComponent>::Get().GetComponent(entity);

    // N/A if not grounded
    if (!actor.newInputs || !HasState(rigidbody._lastCollisionSide, CollisionSide::DOWN))
      continue;

    if (HasState(actor.LastButtons(), InputState::LEFT) || HasState(actor.LastButtons(), InputState::RIGHT))
    {
    }
    else if (!HasState(actor.LastButtons(), InputState::DOWN))
    {
      defer((entity, &state), ActionFactory::GoToNeutralAction(entity, &state));
    }
  }
}

//______________________________________________________________________________
void CheckForMoveLeft::DoTick(float dt)
{
  DeferScopeGuard guard;
  for (const EntityID& entity : Registered)
  {
    Rigidbody& rigidbody = ComponentArray<Rigidbody>::Get().GetComponent(entity);
    GameActor& actor = ComponentArray<GameActor>::Get().GetComponent(entity);
    StateComponent& state = ComponentArray<StateComponent>::Get().GetComponent(entity);

    // N/A if not grounded
    if (!actor.newInputs || !HasState(rigidbody._lastCollisionSide, CollisionSide::DOWN))
      continue;

    if (HasState(actor.LastButtons(), InputState::LEFT))
    {
      Vector2<float> movementVector = Vector2<float>(-0.5f * GlobalVars::BaseWalkSpeed, 0.0f);
      defer((entity, &actor, &state, movementVector), ActionFactory::GoToWalkLeftAction(entity, &actor, &state, movementVector));
    }
  }
}

//______________________________________________________________________________
void CheckForMoveRight::DoTick(float dt)
{
  DeferScopeGuard guard;
  for (const EntityID& entity : Registered)
  {
    Rigidbody& rigidbody = ComponentArray<Rigidbody>::Get().GetComponent(entity);
    GameActor& actor = ComponentArray<GameActor>::Get().GetComponent(entity);
    StateComponent& state = ComponentArray<StateComponent>::Get().GetComponent(entity);

    // N/A if not grounded
    if (!actor.newInputs || !HasState(rigidbody._lastCollisionSide, CollisionSide::DOWN))
      continue;

    if (HasState(actor.LastButtons(), InputState::RIGHT))
    {
      Vector2<float> movementVector = movementVector = Vector2<float>(0.5f * GlobalVars::BaseWalkSpeed, 0.0f);
      defer((entity, &actor, &state, movementVector), ActionFactory::GoToWalkRightAction(entity, &actor, &state, movementVector));
    }
  }
}

//______________________________________________________________________________
void CheckForJump::DoTick(float dt)
{
  DeferScopeGuard guard;
  for (const EntityID& entity : Registered)
  {
    Rigidbody& rigidbody = ComponentArray<Rigidbody>::Get().GetComponent(entity);
    GameActor& actor = ComponentArray<GameActor>::Get().GetComponent(entity);
    StateComponent& state = ComponentArray<StateComponent>::Get().GetComponent(entity);

    // N/A if not grounded
    if (!HasState(rigidbody._lastCollisionSide, CollisionSide::DOWN))
      continue;

    if (HasState(actor.LastButtons(), InputState::UP))
    {
      Vector2<float> movementVector;
      if (HasState(actor.LastButtons(), InputState::LEFT))
        movementVector = Vector2<float>(-0.5f * GlobalVars::BaseWalkSpeed, -UniversalPhysicsSettings::Get().JumpVelocity);
      else if (HasState(actor.LastButtons(), InputState::RIGHT))
        movementVector = Vector2<float>(0.5f * GlobalVars::BaseWalkSpeed, -UniversalPhysicsSettings::Get().JumpVelocity);
      else
        movementVector = Vector2<float>(0.0f, -UniversalPhysicsSettings::Get().JumpVelocity);

      defer((entity, state, movementVector), {
        GameManager::Get().GetEntityByID(entity)->AddComponent<EnactActionComponent>();

        GameManager::Get().GetEntityByID(entity)->AddComponent<MovingActionComponent>();
        GameManager::Get().GetEntityByID(entity)->GetComponent<MovingActionComponent>()->velocity = movementVector;
        GameManager::Get().GetEntityByID(entity)->GetComponent<MovingActionComponent>()->horizontalMovementOnly = false;

        GameManager::Get().GetEntityByID(entity)->AddComponent<AnimatedActionComponent>({ state.onLeftSide, false, false, 1.0f, "Jumping" });

        // transition when fully off the ground
        GameManager::Get().GetEntityByID(entity)->AddComponent<WaitingForJumpAirborne>();

        //
        GameManager::Get().GetEntityByID(entity)->RemoveComponent<AbleToJump>();

        GameManager::Get().GetEntityByID(entity)->RemoveComponent<InputListenerComponent>();
      });
    }
  }
}

//______________________________________________________________________________
void CheckForFalling::DoTick(float dt)
{
  DeferScopeGuard guard;
  for (const EntityID& entity : Registered)
  {
    Rigidbody& rigidbody = ComponentArray<Rigidbody>::Get().GetComponent(entity);
    StateComponent& state = ComponentArray<StateComponent>::Get().GetComponent(entity);

    if (!HasState(rigidbody._lastCollisionSide, CollisionSide::DOWN))
    {
      defer((entity, state), {
        ActionFactory::SetAerialState(entity);
        GameManager::Get().GetEntityByID(entity)->AddComponent<AnimatedActionComponent>({ state.onLeftSide, false, true, 1.0f, "Falling" });
        // because we're falling, remove any movement component
        GameManager::Get().GetEntityByID(entity)->RemoveComponent<MovingActionComponent>();
      });
    }
  }
}

//______________________________________________________________________________
void CheckForBeginCrouching::DoTick(float dt)
{
  DeferScopeGuard guard;
  for (const EntityID& entity : Registered)
  {
    Rigidbody& rigidbody = ComponentArray<Rigidbody>::Get().GetComponent(entity);
    GameActor& actor = ComponentArray<GameActor>::Get().GetComponent(entity);
    StateComponent& state = ComponentArray<StateComponent>::Get().GetComponent(entity);

    // N/A if not grounded
    if (!HasState(rigidbody._lastCollisionSide, CollisionSide::DOWN))
      continue;

    if (HasState(actor.LastButtons(), InputState::DOWN))
    {
      defer((entity, &state), {
        GameManager::Get().GetEntityByID(entity)->AddComponent<EnactActionComponent>();

        GameManager::Get().GetEntityByID(entity)->AddComponent<MovingActionComponent>();
        GameManager::Get().GetEntityByID(entity)->GetComponent<MovingActionComponent>()->velocity = Vector2<float>::Zero;
        GameManager::Get().GetEntityByID(entity)->GetComponent<MovingActionComponent>()->horizontalMovementOnly = true;

        GameManager::Get().GetEntityByID(entity)->AddComponent<AnimatedActionComponent>({ state.onLeftSide, false, false, 1.0f, "Crouching" });
        GameManager::Get().GetEntityByID(entity)->AddComponent<WaitForAnimationComplete>();
        GameManager::Get().GetEntityByID(entity)->AddComponent<TransitionToCrouching>();
        GameManager::Get().GetEntityByID(entity)->AddComponent<CrouchingAction>();

        GameManager::Get().GetEntityByID(entity)->RemoveComponent<AbleToCrouch>();
        GameManager::Get().GetEntityByID(entity)->RemoveComponent<AbleToWalkLeft>();
        GameManager::Get().GetEntityByID(entity)->RemoveComponent<AbleToWalkRight>();
      });
    }
  }
}

//______________________________________________________________________________
void CheckHitThisFrameSystem::DoTick(float dt)
{
  DeferScopeGuard guard;
  for (const EntityID& entity : Registered)
  {
    StateComponent& state = ComponentArray<StateComponent>::Get().GetComponent(entity);
    HittableState& hittable = ComponentArray<HittableState>::Get().GetComponent(entity);
    GameActor& actor = ComponentArray<GameActor>::Get().GetComponent(entity);

    if (state.thrownThisFrame)
    {
      defer((entity, &state), ActionFactory::SetGrappledAction(entity, &state));
      continue;
    }
    else if (state.hitThisFrame)
    {
      // consume the hit this frame flag
      state.hitThisFrame = false;

      InputState const& buttons = actor.LastButtons();
      if (hittable.canBlock)
      {
        bool blockedRight = HasState(buttons, InputState::LEFT) && state.onLeftSide;
        bool blockedLeft = HasState(buttons, InputState::RIGHT) && !state.onLeftSide;
        if (blockedRight || blockedLeft)
        {
          defer((entity, &state, buttons), {
            ActionFactory::SetBlockStunAction(entity, &state, HasState(buttons, InputState::DOWN));
            GameManager::Get().GetEntityByID(entity)->RemoveComponent<InputListenerComponent>();
          });
          continue;
        }
      }

      // if already in knockdown or this attack puts player in knockdown
      if (hittable.inKnockdown || state.hitData.knockdown)
      {
        defer((entity, &state), {
          ActionFactory::SetKnockdownAirborne(entity, &state);
          GameManager::Get().GetEntityByID(entity)->RemoveComponent<InputListenerComponent>();
          });
        continue;
      }

      // default to hitstun state
      defer((entity, &state, buttons), {
        ActionFactory::SetHitStunAction(entity, &state, HasState(buttons, InputState::DOWN));
        GameManager::Get().GetEntityByID(entity)->RemoveComponent<InputListenerComponent>();
      });
    }
  }
}

//______________________________________________________________________________
void CheckSpecialAttackInputSystem::DoTick(float dt)
{
  DeferScopeGuard guard;
  for (const EntityID& entity : Registered)
  {
    StateComponent& state = ComponentArray<StateComponent>::Get().GetComponent(entity);
    GameActor& actor = ComponentArray<GameActor>::Get().GetComponent(entity);

    // only for ryu right now... probably need some kind of move mapping component
    if (HasState(actor.LastButtons(), InputState::BTN1) || HasState(actor.LastButtons(), InputState::BTN2) || HasState(actor.LastButtons(), InputState::BTN3))
    {
      bool fireball = (actor.LastSpecial() == SpecialInputState::QCF && state.onLeftSide) || (actor.LastSpecial() == SpecialInputState::QCB && !state.onLeftSide);
      bool donkeyKick = fireball && (HasState(actor.LastButtons(), InputState::BTN3));
      bool tatsu = (actor.LastSpecial() == SpecialInputState::QCF && !state.onLeftSide) || (actor.LastSpecial() == SpecialInputState::QCB && state.onLeftSide);
      bool dp = (actor.LastSpecial() == SpecialInputState::DPF && state.onLeftSide) || (actor.LastSpecial() == SpecialInputState::DPB && !state.onLeftSide);
      if (fireball || donkeyKick || tatsu || dp)
      {
        if (donkeyKick)
        {
          defer((entity, &state), ActionFactory::SetAttackAction(entity, &state, "SpecialMove3", ActionState::HEAVY));
        }
        else if (fireball)
        {
          defer((entity, &state), ActionFactory::SetAttackAction(entity, &state, "SpecialMove1", ActionState::HEAVY));
        }
        else if (tatsu)
        {
          defer((entity, &state), ActionFactory::SetAttackAction(entity, &state, "SpecialMove4", ActionState::HEAVY));
        }
        else if (dp)
        {
          defer((entity, &state), ActionFactory::SetAttackAction(entity, &state, "SpecialMove2", ActionState::HEAVY));
        }

        defer(entity, {
          // add additional cancelables for special moves here
          // these might not work because they aren't implemented in any systems
          // so they don't generate signatures
          // entity->AddComponents<CancelOnDash, CancelOnJump>();

          // no cancel on hit ground for stuff like tatsu
          GameManager::Get().GetEntityByID(entity)->RemoveComponent<CancelOnHitGround>();
          GameManager::Get().GetEntityByID(entity)->RemoveComponent<CancelOnNormal>();
          GameManager::Get().GetEntityByID(entity)->RemoveComponent<CancelOnSpecial>();
          GameManager::Get().GetEntityByID(entity)->RemoveComponent<CrouchingAction>();

          GameManager::Get().GetEntityByID(entity)->RemoveComponent<InputListenerComponent>();
        });

      }
    }
  }
}

//______________________________________________________________________________
void CheckDashSystem::DoTick(float dt)
{
  DeferScopeGuard guard;
  for (const EntityID& entity : Registered)
  {
    StateComponent& state = ComponentArray<StateComponent>::Get().GetComponent(entity);
    GameActor& actor = ComponentArray<GameActor>::Get().GetComponent(entity);
    Animator& animator = ComponentArray<Animator>::Get().GetComponent(entity);
    Rigidbody& rigidbody = ComponentArray<Rigidbody>::Get().GetComponent(entity);

    if (!HasState(rigidbody._lastCollisionSide, CollisionSide::DOWN))
      continue;

    const InputState& btns = actor.LastButtons();
    const SpecialInputState& sp = actor.LastSpecial();

    if (HasState(btns, InputState::LEFT))
    {
      if (HasState(btns, InputState::BTN4) || sp == SpecialInputState::LDash)
      {
        defer((entity, &state, &animator), ActionFactory::SetDashAction(entity, &state, &animator, !state.onLeftSide));
      }
    }
    else if (HasState(btns, InputState::RIGHT))
    {
      if (HasState(btns, InputState::BTN4) || sp == SpecialInputState::RDash)
      {
        defer((entity, &state, &animator), ActionFactory::SetDashAction(entity, &state, &animator, state.onLeftSide));
      }
    }
  }
}

//______________________________________________________________________________
void CheckAttackInputSystem::DoTick(float dt)
{
  DeferScopeGuard guard;
  for (const EntityID& entity : Registered)
  {
    StateComponent& state = ComponentArray<StateComponent>::Get().GetComponent(entity);
    GameActor& actor = ComponentArray<GameActor>::Get().GetComponent(entity);

    if (HasState(state.collision, CollisionSide::DOWN))
    {
      // first check throws
      if (HasState(actor.LastButtons(), InputState::BTN4))
      {
        bool backthrow = (state.onLeftSide && HasState(actor.LastButtons(), InputState::LEFT)) || (!state.onLeftSide && HasState(actor.LastButtons(), InputState::RIGHT));
        defer((entity, &state, backthrow), ActionFactory::SetAttackAction(entity, &state, backthrow ? "BackThrow" : "ForwardThrow", ActionState::NONE));
        GameManager::Get().GetEntityByID(entity)->AddComponent<GrappleActionComponent>();
        continue;
      }

      bool crouchPossible = HasState(actor.LastButtons(), InputState::DOWN);
      // then check attacks
      if (HasState(actor.LastButtons(), InputState::BTN1))
      {
        defer((entity, &state, crouchPossible), ActionFactory::SetAttackAction(entity, &state, crouchPossible ? "CrouchingLight" : "StandingLight", ActionState::LIGHT));
      }
      else if (HasState(actor.LastButtons(), InputState::BTN2))
      {
        defer((entity, &state, crouchPossible), ActionFactory::SetAttackAction(entity, &state, crouchPossible ? "CrouchingMedium" : "StandingMedium", ActionState::MEDIUM));
      }
      else if (HasState(actor.LastButtons(), InputState::BTN3))
      {
        defer((entity, &state, crouchPossible), ActionFactory::SetAttackAction(entity, &state, crouchPossible ? "CrouchingHeavy" : "StandingHeavy", ActionState::HEAVY));
      }
    }
    else
    {
      if (HasState(actor.LastButtons(), InputState::BTN1))
      {
        defer((entity, &state), ActionFactory::SetAttackAction(entity, &state, "JumpingLight", ActionState::LIGHT));
      }
      else if (HasState(actor.LastButtons(), InputState::BTN2))
      {
        defer((entity, &state), ActionFactory::SetAttackAction(entity, &state, "JumpingMedium", ActionState::MEDIUM));
      }
      else if (HasState(actor.LastButtons(), InputState::BTN3))
      {
        defer((entity, &state), ActionFactory::SetAttackAction(entity, &state, "JumpingHeavy", ActionState::HEAVY));
      }
    }
  }
}

//______________________________________________________________________________
void CheckGrappleCancelOnHit::DoTick(float dt)
{
  DeferScopeGuard guard;
  for (const EntityID& e1 : MainSystem::Registered)
  {
    StateComponent& grapplerState = ComponentArray<StateComponent>::Get().GetComponent(e1);
    if (grapplerState.hitThisFrame)
    {
      for (const EntityID& e2 : SubSystem::Registered)
      {
        Transform& transform = ComponentArray<Transform>::Get().GetComponent(e2);
        StateComponent& state = ComponentArray<StateComponent>::Get().GetComponent(e2);

        state.thrownThisFrame = false;
        defer(e2, GameManager::Get().GetEntityByID(e2)->RemoveComponent<ReceivedGrappleAction>());
      }
    }
  }
}

//______________________________________________________________________________
void ListenForAirborneSystem::DoTick(float dt)
{
  DeferScopeGuard guard;
  for (const EntityID& entity : Registered)
  {
    Rigidbody& rigidbody = ComponentArray<Rigidbody>::Get().GetComponent(entity);
    if (!HasState(rigidbody._lastCollisionSide, CollisionSide::DOWN))
    {
      defer(entity, ActionFactory::SetAerialState(entity));
    }
  }
}

//______________________________________________________________________________
void TransitionToNeutralSystem::DoTick(float dt)
{
  DeferScopeGuard guard;
  for (const EntityID& entity : Registered)
  {
    StateComponent& state = ComponentArray<StateComponent>::Get().GetComponent(entity);
    GameActor& actor = ComponentArray<GameActor>::Get().GetComponent(entity);
    Rigidbody& rigidbody = ComponentArray<Rigidbody>::Get().GetComponent(entity);

    if (actor.actionTimerComplete)
    {
      defer((entity, &state), ActionFactory::GoToNeutralAction(entity, &state));
      if (HasState(actor.LastButtons(), InputState::DOWN) && HasState(rigidbody._lastCollisionSide, CollisionSide::DOWN))
      {
        defer((entity, &state), ActionFactory::SetCrouchingState(entity, &state));
      }
      else if (!HasState(rigidbody._lastCollisionSide, CollisionSide::DOWN))
      {
        //ActionFactory::SetAerialState(entity);
      }
    }
  }
}

//______________________________________________________________________________
void CheckKnockdownComplete::DoTick(float dt)
{
  DeferScopeGuard guard;
  for (const EntityID& entity : Registered)
  {
    StateComponent& state = ComponentArray<StateComponent>::Get().GetComponent(entity);
    GameActor& actor = ComponentArray<GameActor>::Get().GetComponent(entity);

    if (actor.actionTimerComplete)
    {
      defer((entity, &state), ActionFactory::SetKnockdownGroundInvincible(entity, &state));
    }
  }
}

//______________________________________________________________________________
void CheckKnockdownOTG::DoTick(float dt)
{
  DeferScopeGuard guard;
  for (const EntityID& entity : Registered)
  {
    StateComponent& state = ComponentArray<StateComponent>::Get().GetComponent(entity);
    Rigidbody& rigidbody = ComponentArray<Rigidbody>::Get().GetComponent(entity);

    if (HasState(rigidbody._lastCollisionSide, CollisionSide::DOWN))
    {
      defer((entity, &state), ActionFactory::SetKnockdownGroundOTG(entity, &state));
    }
  }
}

//______________________________________________________________________________
void CheckCrouchingFollowUp::DoTick(float dt)
{
  DeferScopeGuard guard;
  for (const EntityID& entity : Registered)
  {
    StateComponent& state = ComponentArray<StateComponent>::Get().GetComponent(entity);
    GameActor& actor = ComponentArray<GameActor>::Get().GetComponent(entity);

    if (actor.actionTimerComplete)
    {
      defer((entity, &state), ActionFactory::SetCrouchingState(entity, &state));
    }
  }
}

//______________________________________________________________________________
void HitGroundCancelActionSystem::DoTick(float dt)
{
  for (const EntityID& entity : Registered)
  {
    Rigidbody& rb = ComponentArray<Rigidbody>::Get().GetComponent(entity);
    GameActor& actor = ComponentArray<GameActor>::Get().GetComponent(entity);
    if (HasState(rb._lastCollisionSide, CollisionSide::DOWN))
    {
      actor.actionTimerComplete = true;
    }
  }
}

//______________________________________________________________________________
void SpecialMoveCancelActionSystem::DoTick(float dt)
{
  DeferScopeGuard guard;
  for (const EntityID& entity : Registered)
  {
    GameActor& actor = ComponentArray<GameActor>::Get().GetComponent(entity);
    StateComponent& state = ComponentArray<StateComponent>::Get().GetComponent(entity);

    // can only be cancelled on hit for now (replace with HittingComponent maybe?)
    if(state.hitting && ActionFactory::ActorDidSpecialInputRyu(&actor, &state))
    {
      defer(entity, {
        GameManager::Get().GetEntityByID(entity)->AddComponent<AbleToSpecialAttackState>();
        GameManager::Get().GetEntityByID(entity)->RemoveComponent<CancelOnSpecial>();
        });
    }
  }
}

//______________________________________________________________________________
void TargetComboCancelActionSystem::DoTick(float dt)
{
  DeferScopeGuard guard;
  for (const EntityID& entity : Registered)
  {
    GameActor& actor = ComponentArray<GameActor>::Get().GetComponent(entity);
    StateComponent& state = ComponentArray<StateComponent>::Get().GetComponent(entity);
    HasTargetCombo& comboableMap = ComponentArray<HasTargetCombo>::Get().GetComponent(entity);

    // can only be cancelled on hit for now (replace with HittingComponent maybe?)
    if (state.hitting)
    {
      // if has a combo for this state and player is pressing that corresponding input
      if (comboableMap.links.find(state.actionState) != comboableMap.links.end())
      {
        if (comboableMap.links[state.actionState] == actor.LastButtons())
        {
          defer(entity, {
            GameManager::Get().GetEntityByID(entity)->AddComponent<AbleToAttackState>();
            GameManager::Get().GetEntityByID(entity)->RemoveComponent<CancelOnNormal>();
          });
        }
      }
    }
  }
}
