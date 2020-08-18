#include "Core/Prefab/ActionFactory.h"

#include "Components/ActionComponents.h"
#include "Components/Animator.h"
#include "Components/StateComponents/AttackStateComponent.h"
#include "Components/StateComponents/HitStateComponent.h"
#include "Components/Actors/GameActor.h"

#include "Managers/GameManagement.h"

void ActionFactory::SetAerialState(const EntityID& entity)
{
  GameManager::Get().GetEntityByID(entity)->RemoveComponents<AbleToJump, AbleToCrouch, AbleToDash, AbleToWalkLeft, AbleToWalkRight>();
  GameManager::Get().GetEntityByID(entity)->AddComponent<JumpingAction>();
}

void ActionFactory::SetCrouchingState(const EntityID& entity, StateComponent* state)
{
  GameManager::Get().GetEntityByID(entity)->AddComponent<AnimatedActionComponent>({ state->onLeftSide, true, true, 1.0f, "Crouch" });
  GameManager::Get().GetEntityByID(entity)->AddComponent<EnactActionComponent>();
  GameManager::Get().GetEntityByID(entity)->AddComponent<AbleToReturnToNeutral>();
  GameManager::Get().GetEntityByID(entity)->RemoveComponents<TransitionToCrouching, AbleToWalkLeft, AbleToWalkRight>();
}

void ActionFactory::SetKnockdownAirborne(const EntityID& entity, StateComponent* state)
{
  RemoveTransitionComponents(entity);
  DisableAbility(entity);

  state->actionState = ActionState::HITSTUN;

  // Always reset action complete flag on new action
  GameManager::Get().GetEntityByID(entity)->GetComponent<GameActor>()->actionTimerComplete = false;

  // set up knockdown air action
  GameManager::Get().GetEntityByID(entity)->AddComponent<AnimatedActionComponent>({ state->onLeftSide, false, true, 1.0f, "Knockdown_Air" });

  // set up movement action for knockback
  GameManager::Get().GetEntityByID(entity)->AddComponent<MovingActionComponent>();
  GameManager::Get().GetEntityByID(entity)->GetComponent<MovingActionComponent>()->velocity = state->hitData.knockback;
  GameManager::Get().GetEntityByID(entity)->GetComponent<MovingActionComponent>()->horizontalMovementOnly = false;

  // set empty component for follow up action
  GameManager::Get().GetEntityByID(entity)->AddComponent<TransitionToKnockdownGroundOTG>();;

  // add hittable because you can still be hit while in the air
  GameManager::Get().GetEntityByID(entity)->AddComponent<HittableState>();
  GameManager::Get().GetEntityByID(entity)->GetComponent<HittableState>()->canBlock = false;
  GameManager::Get().GetEntityByID(entity)->GetComponent<HittableState>()->inKnockdown = true;

  GameManager::Get().GetEntityByID(entity)->AddComponent<ReceivedDamageAction>();
  GameManager::Get().GetEntityByID(entity)->GetComponent<ReceivedDamageAction>()->damageAmount = state->hitData.damage;
  GameManager::Get().GetEntityByID(entity)->GetComponent<ReceivedDamageAction>()->isBlocking = false;

  GameManager::Get().GetEntityByID(entity)->AddComponent<EnactActionComponent>();

  // we are in a juggle state here
  GameManager::Get().GetEntityByID(entity)->GetComponent<Gravity>()->force = GlobalVars::JuggleGravity;

  // in case we still have a grapple state attached
  GameManager::Get().GetEntityByID(entity)->RemoveComponent<ReceivedGrappleAction>();
}

void ActionFactory::SetKnockdownGroundOTG(const EntityID& entity, StateComponent* state)
{
  RemoveTransitionComponents(entity);
  DisableAbility(entity);

  state->actionState = ActionState::HITSTUN;

  // Always reset action complete flag on new action
  GameManager::Get().GetEntityByID(entity)->GetComponent<GameActor>()->actionTimerComplete = false;

  // set up knockdown air action
  GameManager::Get().GetEntityByID(entity)->AddComponent<AnimatedActionComponent>({ state->onLeftSide, false, false, 1.0f, "Knockdown_HitGround" });

  // set up movement action for knockback
  GameManager::Get().GetEntityByID(entity)->AddComponent<MovingActionComponent>();
  GameManager::Get().GetEntityByID(entity)->GetComponent<MovingActionComponent>()->velocity = Vector2<float>::Zero;
  GameManager::Get().GetEntityByID(entity)->GetComponent<MovingActionComponent>()->horizontalMovementOnly = true;

  // freeze in this state until animation is done
  GameManager::Get().GetEntityByID(entity)->AddComponent<WaitForAnimationComplete>();

  // set empty component for follow up action
  GameManager::Get().GetEntityByID(entity)->AddComponent<TransitionToKnockdownGround>();

  // add hittable because you will be able to OTG during this period
  GameManager::Get().GetEntityByID(entity)->AddComponent<HittableState>();
  GameManager::Get().GetEntityByID(entity)->GetComponent<HittableState>()->canBlock = false;
  GameManager::Get().GetEntityByID(entity)->GetComponent<HittableState>()->inKnockdown = true;

  GameManager::Get().GetEntityByID(entity)->AddComponent<EnactActionComponent>();

  // remove stuff from last state
  GameManager::Get().GetEntityByID(entity)->RemoveComponent<TransitionToKnockdownGroundOTG>();
  GameManager::Get().GetEntityByID(entity)->RemoveComponent<ReceivedDamageAction>();
  GameManager::Get().GetEntityByID(entity)->RemoveComponent<ReceivedGrappleAction>();
}

void ActionFactory::SetKnockdownGroundInvincible(const EntityID& entity, StateComponent* state)
{
  RemoveTransitionComponents(entity);
  DisableAbility(entity);

  state->actionState = ActionState::NONE;

  // Always reset action complete flag on new action
  GameManager::Get().GetEntityByID(entity)->GetComponent<GameActor>()->actionTimerComplete = false;

  // set up knockdown air action
  GameManager::Get().GetEntityByID(entity)->AddComponent<AnimatedActionComponent>({ state->onLeftSide, false, false, 1.0f, "Knockdown_OnGround" });

  // set up movement action for knockback
  GameManager::Get().GetEntityByID(entity)->AddComponent<MovingActionComponent>();
  GameManager::Get().GetEntityByID(entity)->GetComponent<MovingActionComponent>()->velocity = Vector2<float>::Zero;

  // freeze in this state until animation is done
  GameManager::Get().GetEntityByID(entity)->AddComponent<WaitForAnimationComplete>();

  // set empty component for follow up action
  GameManager::Get().GetEntityByID(entity)->AddComponent<TransitionToNeutral>();

  GameManager::Get().GetEntityByID(entity)->AddComponent<EnactActionComponent>();

  // remove attack components cause am in hitstun
  GameManager::Get().GetEntityByID(entity)->RemoveComponent<AbleToAttackState>();
  GameManager::Get().GetEntityByID(entity)->RemoveComponent<AbleToSpecialAttackState>();

  // not hittable during this period
  GameManager::Get().GetEntityByID(entity)->RemoveComponent<HittableState>();

  GameManager::Get().GetEntityByID(entity)->RemoveComponent<TransitionToKnockdownGround>();

  // reset juggle state here
  GameManager::Get().GetEntityByID(entity)->GetComponent<Gravity>()->force = GlobalVars::Gravity;
}

void ActionFactory::SetBlockStunAction(const EntityID& entity, StateComponent* state, bool crouching)
{
  RemoveTransitionComponents(entity);
  DisableAbility(entity);

  state->actionState = ActionState::BLOCKSTUN;

  // Always reset action complete flag on new action
  GameManager::Get().GetEntityByID(entity)->GetComponent<GameActor>()->actionTimerComplete = false;

  // set up knockdown air action
  GameManager::Get().GetEntityByID(entity)->AddComponent<AnimatedActionComponent>({ state->onLeftSide, false, true, 1.0f, crouching ? "BlockLow" : "BlockMid" });

  if (HasState(state->collision, CollisionSide::DOWN))
  {
    // set up movement action for stopping movement
    GameManager::Get().GetEntityByID(entity)->AddComponent<MovingActionComponent>();
    GameManager::Get().GetEntityByID(entity)->GetComponent<MovingActionComponent>()->velocity = Vector2<float>::Zero;
    GameManager::Get().GetEntityByID(entity)->GetComponent<MovingActionComponent>()->horizontalMovementOnly = true;
  }

  // freeze in this state 
  GameManager::Get().GetEntityByID(entity)->AddComponent<TimedActionComponent>();
  GameManager::Get().GetEntityByID(entity)->GetComponent<TimedActionComponent>()->totalFrames = state->hitData.framesInStunBlock;

  // set empty component for follow up action
  GameManager::Get().GetEntityByID(entity)->AddComponent<TransitionToNeutral>();

  // add states for potential inputs
  GameManager::Get().GetEntityByID(entity)->AddComponent<HittableState>();
  GameManager::Get().GetEntityByID(entity)->GetComponent<HittableState>()->canBlock = true;
  GameManager::Get().GetEntityByID(entity)->GetComponent<HittableState>()->inKnockdown = false;

  GameManager::Get().GetEntityByID(entity)->AddComponent<ReceivedDamageAction>();
  GameManager::Get().GetEntityByID(entity)->GetComponent<ReceivedDamageAction>()->damageAmount = 0;
  GameManager::Get().GetEntityByID(entity)->GetComponent<ReceivedDamageAction>()->isBlocking = true;

  GameManager::Get().GetEntityByID(entity)->AddComponent<EnactActionComponent>();

  GameManager::Get().GetEntityByID(entity)->AddComponent<HitStateComponent>();
  GameManager::Get().GetEntityByID(entity)->GetComponent<HitStateComponent>()->SetTimer(GameManager::Get().GetEntityByID(entity)->GetComponent<TimedActionComponent>());
}

void ActionFactory::SetHitStunAction(const EntityID& entity, StateComponent* state, bool crouching)
{
  RemoveTransitionComponents(entity);
  DisableAbility(entity);

  state->actionState = ActionState::HITSTUN;

  // Always reset action complete flag on new action
  GameManager::Get().GetEntityByID(entity)->GetComponent<GameActor>()->actionTimerComplete = false;

  std::string hitstunAnim = "LightHitstun";
  if (state->hitData.framesInStunHit > 20) hitstunAnim = "MedHitstun";
  if (state->hitData.framesInStunHit > 30) hitstunAnim = "HeavyHitstun";

  // set up animation
  GameManager::Get().GetEntityByID(entity)->AddComponent<AnimatedActionComponent>({ state->onLeftSide, false, true, 1.0f, (crouching && state->hitData.framesInStunHit < 35) ? "CrouchingHitstun" : hitstunAnim });

  // set up movement action for knockback
  GameManager::Get().GetEntityByID(entity)->AddComponent<MovingActionComponent>();
  GameManager::Get().GetEntityByID(entity)->GetComponent<MovingActionComponent>()->velocity = state->hitData.knockback;
  GameManager::Get().GetEntityByID(entity)->GetComponent<MovingActionComponent>()->horizontalMovementOnly = false;

  // freeze in this state 
  GameManager::Get().GetEntityByID(entity)->AddComponent<TimedActionComponent>();
  GameManager::Get().GetEntityByID(entity)->GetComponent<TimedActionComponent>()->totalFrames = state->hitData.framesInStunHit;

  // set empty component for follow up action
  GameManager::Get().GetEntityByID(entity)->AddComponent<TransitionToNeutral>();

  GameManager::Get().GetEntityByID(entity)->AddComponent<ReceivedDamageAction>();
  GameManager::Get().GetEntityByID(entity)->GetComponent<ReceivedDamageAction>()->damageAmount = state->hitData.damage;
  GameManager::Get().GetEntityByID(entity)->GetComponent<ReceivedDamageAction>()->isBlocking = false;

  // add states for potential inputs
  GameManager::Get().GetEntityByID(entity)->AddComponent<HittableState>();
  GameManager::Get().GetEntityByID(entity)->GetComponent<HittableState>()->canBlock = false;
  GameManager::Get().GetEntityByID(entity)->GetComponent<HittableState>()->inKnockdown = false;

  GameManager::Get().GetEntityByID(entity)->AddComponent<EnactActionComponent>();

  GameManager::Get().GetEntityByID(entity)->AddComponent<HitStateComponent>();
  GameManager::Get().GetEntityByID(entity)->GetComponent<HitStateComponent>()->SetTimer(GameManager::Get().GetEntityByID(entity)->GetComponent<TimedActionComponent>());
}

void ActionFactory::SetGrappledAction(const EntityID& entity, StateComponent* state)
{
  RemoveTransitionComponents(entity);
  DisableAbility(entity);

  // Always reset action complete flag on new action
  GameManager::Get().GetEntityByID(entity)->GetComponent<GameActor>()->actionTimerComplete = false;

  // set up animation
  GameManager::Get().GetEntityByID(entity)->AddComponent<AnimatedActionComponent>({ state->onLeftSide, false, true, 1.0f, "HeavyHitstun" });

  GameManager::Get().GetEntityByID(entity)->AddComponent<ReceivedGrappleAction>();
  GameManager::Get().GetEntityByID(entity)->GetComponent<ReceivedGrappleAction>()->damageAndKnockbackDelay = state->hitData.activeFrames;
  GameManager::Get().GetEntityByID(entity)->GetComponent<ReceivedGrappleAction>()->damageAmount = state->hitData.damage;

  // freeze in this state til throw is initiated
  GameManager::Get().GetEntityByID(entity)->AddComponent<TimedActionComponent>();
  GameManager::Get().GetEntityByID(entity)->GetComponent<TimedActionComponent>()->totalFrames = state->hitData.activeFrames;

  GameManager::Get().GetEntityByID(entity)->AddComponent<EnactActionComponent>();
}


void ActionFactory::SetAttackAction(const EntityID& entity, StateComponent* state, const std::string& attackName, ActionState actionType)
{
  RemoveTransitionComponents(entity);
  DisableAbility(entity);

  // resetting state component... i dont really like this
  state->hitting = false;
  // mark action type here
  state->actionState = actionType;

  // Always reset action complete flag on new action
  GameManager::Get().GetEntityByID(entity)->GetComponent<GameActor>()->actionTimerComplete = false;

  // set up animation
  GameManager::Get().GetEntityByID(entity)->AddComponent<AnimatedActionComponent>({ state->onLeftSide, false, true, 1.0f, attackName });

  // since this is an animation locked attack
  GameManager::Get().GetEntityByID(entity)->AddComponent<WaitForAnimationComplete>();

  GameManager::Get().GetEntityByID(entity)->AddComponent<AttackActionComponent>();
  GameManager::Get().GetEntityByID(entity)->GetComponent<AttackActionComponent>()->type = actionType;

  // NOTE: THIS IS BUGGED WITH MOVES THAT HAVE UPWARDS MOVEMENT ON THE FIRST FRAME
  // THIS IS BECAUSE THE MOVEMENT FUNCTIONS FOR MOVES RUN THROUGH THE GENERIC TIMER SYSTEM
  // NEED TO FIX THIS - ORDERING IS A PAIN
  if (HasState(state->collision, CollisionSide::DOWN))
  {
    // set up movement action for stopping movement
    GameManager::Get().GetEntityByID(entity)->AddComponent<MovingActionComponent>();
    GameManager::Get().GetEntityByID(entity)->GetComponent<MovingActionComponent>()->velocity = Vector2<float>::Zero;
    GameManager::Get().GetEntityByID(entity)->GetComponent<MovingActionComponent>()->horizontalMovementOnly = true;
  }

  // add states for potential inputs
  GameManager::Get().GetEntityByID(entity)->AddComponent<HittableState>();
  GameManager::Get().GetEntityByID(entity)->GetComponent<HittableState>()->canBlock = false;
  GameManager::Get().GetEntityByID(entity)->GetComponent<HittableState>()->inKnockdown = false;

  // set empty component for follow up action
  GameManager::Get().GetEntityByID(entity)->AddComponent<TransitionToNeutral>();

  // add cancels here
  GameManager::Get().GetEntityByID(entity)->AddComponents<CancelOnHitGround, CancelOnSpecial, CancelOnNormal>();

  GameManager::Get().GetEntityByID(entity)->AddComponent<EnactActionComponent>();
}

void ActionFactory::SetDashAction(const EntityID& entity, StateComponent* state, Animator* animator, bool dashDirectionForward)
{
  RemoveTransitionComponents(entity);
  DisableAbility(entity);

  // Always reset action complete flag on new action
  GameManager::Get().GetEntityByID(entity)->GetComponent<GameActor>()->actionTimerComplete = false;

  std::string animationName = dashDirectionForward ? "ForwardDash" : "BackDash";
  // set up animation
  float dashPlaySpeed = static_cast<float>(animator->AnimationLib()->GetAnimation(animationName)->GetFrameCount()) / static_cast<float>(GlobalVars::nDashFrames);
  GameManager::Get().GetEntityByID(entity)->AddComponent<AnimatedActionComponent>({ state->onLeftSide, false, true, dashPlaySpeed, animationName });

  GameManager::Get().GetEntityByID(entity)->AddComponent<DashingAction>();
  GameManager::Get().GetEntityByID(entity)->GetComponent<DashingAction>()->dashSpeed = GlobalVars::BaseWalkSpeed * 1.5f;
  if ((state->onLeftSide && !dashDirectionForward) || (!state->onLeftSide && dashDirectionForward))
    GameManager::Get().GetEntityByID(entity)->GetComponent<DashingAction>()->dashSpeed *= -1.0f;

  // set up the end timer
  GameManager::Get().GetEntityByID(entity)->AddComponent<TimedActionComponent>();
  GameManager::Get().GetEntityByID(entity)->GetComponent<TimedActionComponent>()->totalFrames = GlobalVars::nDashFrames;


  // add states for potential inputs
  GameManager::Get().GetEntityByID(entity)->AddComponent<HittableState>();
  GameManager::Get().GetEntityByID(entity)->GetComponent<HittableState>()->canBlock = false;
  GameManager::Get().GetEntityByID(entity)->GetComponent<HittableState>()->inKnockdown = false;

  // set empty component for follow up action
  GameManager::Get().GetEntityByID(entity)->AddComponent<TransitionToNeutral>();

  GameManager::Get().GetEntityByID(entity)->AddComponent<EnactActionComponent>();

  GameManager::Get().GetEntityByID(entity)->RemoveComponent<AbleToDash>();
}

void ActionFactory::GoToNeutralAction(const EntityID& entity, StateComponent* state)
{
  ResetActionComponents(entity);
  // Always reset action complete flag on new action
  GameManager::Get().GetEntityByID(entity)->GetComponent<GameActor>()->actionTimerComplete = false;

  GameManager::Get().GetEntityByID(entity)->AddComponent<EnactActionComponent>();
  GameManager::Get().GetEntityByID(entity)->AddComponent<AnimatedActionComponent>({ state->onLeftSide, true, false, 1.0f, "Idle" });

  // set up movement action for stopping movement
  GameManager::Get().GetEntityByID(entity)->AddComponent<MovingActionComponent>();
  GameManager::Get().GetEntityByID(entity)->GetComponent<MovingActionComponent>()->velocity = Vector2<float>::Zero;
  GameManager::Get().GetEntityByID(entity)->GetComponent<MovingActionComponent>()->horizontalMovementOnly = true;

  // add states for potential outside influence
  GameManager::Get().GetEntityByID(entity)->AddComponent<HittableState>();
  GameManager::Get().GetEntityByID(entity)->GetComponent<HittableState>()->canBlock = true;
  GameManager::Get().GetEntityByID(entity)->GetComponent<HittableState>()->inKnockdown = false;

  // in neutral, all of these options are available, so actions can be accessed via inputs
  EnableAbility(entity);

  GameManager::Get().GetEntityByID(entity)->RemoveComponents<DashingAction, AbleToReturnToNeutral>();

  state->actionState = ActionState::NONE;
}

void ActionFactory::GoToWalkLeftAction(const EntityID& entity, GameActor* actor, StateComponent* state, const Vector2<float>& mvmt)
{
  // Always reset action complete flag on new action
  actor->actionTimerComplete = false;

  GameManager::Get().GetEntityByID(entity)->AddComponents<EnactActionComponent, MovingActionComponent, HittableState, AbleToReturnToNeutral>();

  // set animation based on direction
  std::string animation = state->onLeftSide ? "WalkB" : "WalkF";
  GameManager::Get().GetEntityByID(entity)->AddComponent<AnimatedActionComponent>({ state->onLeftSide, true, false, 1.0f, animation });

  GameManager::Get().GetEntityByID(entity)->GetComponent<MovingActionComponent>()->velocity = mvmt;
  GameManager::Get().GetEntityByID(entity)->GetComponent<MovingActionComponent>()->horizontalMovementOnly = true;

  // add states for potential outside influence
  GameManager::Get().GetEntityByID(entity)->AddComponent<>();
  GameManager::Get().GetEntityByID(entity)->GetComponent<HittableState>()->canBlock = true;
  GameManager::Get().GetEntityByID(entity)->GetComponent<HittableState>()->inKnockdown = false;

  // enable all abilities in neutral
  ActionFactory::EnableAbility(entity);

  GameManager::Get().GetEntityByID(entity)->RemoveComponents<InputListenerComponent, AbleToWalkLeft>();
}

void ActionFactory::GoToWalkRightAction(const EntityID& entity, GameActor* actor, StateComponent* state, const Vector2<float>& mvmt)
{
  // Always reset action complete flag on new action
  actor->actionTimerComplete = false;

  GameManager::Get().GetEntityByID(entity)->AddComponents<EnactActionComponent, MovingActionComponent, HittableState, AbleToReturnToNeutral>();

  // set animation based on direction
  std::string animation = state->onLeftSide ? "WalkF" : "WalkB";
  GameManager::Get().GetEntityByID(entity)->AddComponent<AnimatedActionComponent>({ state->onLeftSide, true, false, 1.0f, animation });

  GameManager::Get().GetEntityByID(entity)->GetComponent<MovingActionComponent>()->velocity = mvmt;
  GameManager::Get().GetEntityByID(entity)->GetComponent<MovingActionComponent>()->horizontalMovementOnly = true;

  // add states for potential outside influence
  GameManager::Get().GetEntityByID(entity)->AddComponent<>();
  GameManager::Get().GetEntityByID(entity)->GetComponent<HittableState>()->canBlock = true;
  GameManager::Get().GetEntityByID(entity)->GetComponent<HittableState>()->inKnockdown = false;

  // enable all abilities in neutral
  ActionFactory::EnableAbility(entity);

  GameManager::Get().GetEntityByID(entity)->RemoveComponents<InputListenerComponent, AbleToWalkRight>();
}

void ActionFactory::ResetActionComponents(const EntityID& entity)
{
  // remove all state dependent components (hopefully this doesn't fuck performance)
  RemoveTransitionComponents(entity);

  GameManager::Get().GetEntityByID(entity)->RemoveComponents<CancelOnHitGround, CancelOnDash, CancelOnJump, CancelOnSpecial, CancelOnNormal>();

  // these components should be merged....
  GameManager::Get().GetEntityByID(entity)->RemoveComponent<AttackActionComponent>();
  GameManager::Get().GetEntityByID(entity)->RemoveComponent<AttackStateComponent>();

  GameManager::Get().GetEntityByID(entity)->RemoveComponent<GrappleActionComponent>();
  GameManager::Get().GetEntityByID(entity)->RemoveComponent<MovingActionComponent>();
  GameManager::Get().GetEntityByID(entity)->RemoveComponent<ReceivedDamageAction>();
  GameManager::Get().GetEntityByID(entity)->RemoveComponent<ReceivedGrappleAction>();
  GameManager::Get().GetEntityByID(entity)->RemoveComponent<DashingAction>();

  GameManager::Get().GetEntityByID(entity)->AddComponent<InputListenerComponent>();
  
}

void ActionFactory::RemoveTransitionComponents(const EntityID& entity)
{
  GameManager::Get().GetEntityByID(entity)->RemoveComponent<TransitionToKnockdownGround>();
  GameManager::Get().GetEntityByID(entity)->RemoveComponent<TransitionToKnockdownGroundOTG>();
  GameManager::Get().GetEntityByID(entity)->RemoveComponent<TransitionToNeutral>();
  GameManager::Get().GetEntityByID(entity)->RemoveComponent<TransitionToCrouching>();

  GameManager::Get().GetEntityByID(entity)->RemoveComponent<TimedActionComponent>();
  GameManager::Get().GetEntityByID(entity)->RemoveComponent<WaitForAnimationComplete>();
  GameManager::Get().GetEntityByID(entity)->RemoveComponent<WaitingForJumpAirborne>();
  GameManager::Get().GetEntityByID(entity)->RemoveComponent<MovingActionComponent>();

  GameManager::Get().GetEntityByID(entity)->RemoveComponent<AttackStateComponent>();
  GameManager::Get().GetEntityByID(entity)->RemoveComponent<HitStateComponent>();

  // remove all action state components
  //GameManager::Get().GetEntityByID(entity)->RemoveComponents<DashingAction, JumpingAction, CrouchingAction>();
}


void ActionFactory::DisableAbility(const EntityID& entity)
{
  GameManager::Get().GetEntityByID(entity)->RemoveComponents<AbleToAttackState, AbleToSpecialAttackState, AbleToDash, AbleToJump, AbleToWalkLeft, AbleToWalkRight, AbleToCrouch, AbleToReturnToNeutral>();
}

void ActionFactory::EnableAbility(const EntityID& entity)
{
  // add things that give control to player
  GameManager::Get().GetEntityByID(entity)->AddComponents<AbleToAttackState, AbleToSpecialAttackState, AbleToDash, AbleToJump, AbleToWalkLeft, AbleToWalkRight, AbleToCrouch>();

  // remove things that take control away from player
  GameManager::Get().GetEntityByID(entity)->RemoveComponent<WallPushComponent>();
}

bool ActionFactory::ActorDidSpecialInputRyu(GameActor* actor, StateComponent* state)
{
  // only for ryu right now... probably need some kind of move mapping component
  if (HasState(actor->LastButtons(), InputState::BTN1) || HasState(actor->LastButtons(), InputState::BTN2) || HasState(actor->LastButtons(), InputState::BTN3))
  {
    bool fireball = (actor->LastSpecial() == SpecialInputState::QCF && state->onLeftSide) || (actor->LastSpecial() == SpecialInputState::QCB && !state->onLeftSide);
    bool donkeyKick = fireball && (HasState(actor->LastButtons(), InputState::BTN3));
    bool tatsu = (actor->LastSpecial() == SpecialInputState::QCF && !state->onLeftSide) || (actor->LastSpecial() == SpecialInputState::QCB && state->onLeftSide);
    bool dp = (actor->LastSpecial() == SpecialInputState::DPF && state->onLeftSide) || (actor->LastSpecial() == SpecialInputState::DPB && !state->onLeftSide);

    return fireball || donkeyKick || tatsu || dp;
  }
  return false;
}
