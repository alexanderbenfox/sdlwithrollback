#include "Systems/FighterFSMSystem.h"

#include "Components/RenderComponent.h"
#include "Components/SFXComponent.h"
#include "Components/StateComponents/AttackStateComponent.h"
#include "Components/StateComponents/HitStateComponent.h"
#include "Components/ActionComponents.h"
#include "Components/StaticComponents/AttackLinkMap.h"

#include "Managers/GameManagement.h"
#include "Managers/AnimationCollectionManager.h"

//______________________________________________________________________________
void FighterFSMSystem::DoTick(float dt)
{
  PROFILE_FUNCTION();

  // No dt <= 0 early return: the FSM must evaluate transitions during hitstop
  // (dt=0) so that cancel windows extend through the freeze, matching how the
  // old action systems behaved. Timer advancement and animation accumulation
  // naturally handle dt=0 (they add 0, so nothing advances).

  for (const EntityID& entity : Registered)
  {
    FighterFSMComponent& fsm = ComponentArray<FighterFSMComponent>::Get().GetComponent(entity);
    GameActor& actor = ComponentArray<GameActor>::Get().GetComponent(entity);
    StateComponent& state = ComponentArray<StateComponent>::Get().GetComponent(entity);
    Rigidbody& rb = ComponentArray<Rigidbody>::Get().GetComponent(entity);
    Animator& animator = ComponentArray<Animator>::Get().GetComponent(entity);

    // 1. If state was just entered, execute entry actions
    if (fsm.stateJustEntered)
    {
      EnactState(entity, fsm, actor, state, rb, animator);
      fsm.stateJustEntered = false;
    }

    // 2. Latch the hitting flag: once state.hitting goes true, hitConfirmed
    // stays true until the next state transition clears it. This ensures the
    // cancel window survives through hitstop (when HitSystem skips and would
    // otherwise clear hitting before the FSM can check it).
    if (state.hitting)
      fsm.hitConfirmed = true;

    // 3. Update current state (advance timer, dash velocity)
    UpdateCurrentState(dt, entity, fsm, state, rb);

    // 4. Build condition flags from current entity state
    uint32_t flags = EvaluateConditionFlags(fsm, actor, state, rb, animator);

    // 4. Evaluate transitions — first matching rule wins
    FighterStateID target = EvaluateTransitions(entity, fsm, flags, actor, state, rb);

    // 5. Throw whiff check: if in a throw state and throw was attempted but missed
    if (target == fsm.currentState &&
        (fsm.currentState == FighterStateID::ForwardThrow || fsm.currentState == FighterStateID::BackThrow))
    {
      if (state.triedToThrowThisFrame && !state.throwSuccess)
        target = FighterStateID::ThrowMiss;
    }

    // 6. If a transition fires, execute it
    // Allow re-entry into the same state on hit/thrown (e.g., getting hit while in hitstun
    // must reset the timer and re-apply damage)
    bool shouldTransition = (target != fsm.currentState);
    if (!shouldTransition && (flags & (CF_HitThisFrame | CF_ThrownThisFrame)))
      shouldTransition = true;

    if (shouldTransition)
    {
      TransitionTo(entity, fsm, target, state, rb);
    }
  }

  // Post-loop: grapple cancel check
  // If any entity that was throwing got hit, free the grappled entity
  for (const EntityID& entity : Registered)
  {
    FighterFSMComponent& fsm = ComponentArray<FighterFSMComponent>::Get().GetComponent(entity);

    // Check if this entity just left a throw state due to being hit
    bool wasInThrow = (fsm.previousState == FighterStateID::ForwardThrow ||
                       fsm.previousState == FighterStateID::BackThrow);
    bool isInDamageState = (fsm.currentState == FighterStateID::Hitstun ||
                            fsm.currentState == FighterStateID::CrouchingHitstun ||
                            fsm.currentState == FighterStateID::BlockstunStanding ||
                            fsm.currentState == FighterStateID::BlockstunCrouching ||
                            fsm.currentState == FighterStateID::KnockdownAirborne);

    if (wasInThrow && isInDamageState && fsm.stateJustEntered)
    {
      // Thrower got hit — free all grappled entities
      for (const EntityID& other : Registered)
      {
        if (other == entity) continue;
        FighterFSMComponent& otherFsm = ComponentArray<FighterFSMComponent>::Get().GetComponent(other);
        if (otherFsm.currentState == FighterStateID::Grappled)
        {
          StateComponent& otherState = ComponentArray<StateComponent>::Get().GetComponent(other);
          Rigidbody& otherRb = ComponentArray<Rigidbody>::Get().GetComponent(other);
          otherState.thrownThisFrame = false;
          TransitionTo(other, otherFsm, FighterStateID::Idle, otherState, otherRb);
        }
      }
    }
  }
}

//______________________________________________________________________________
uint32_t FighterFSMSystem::EvaluateConditionFlags(
  const FighterFSMComponent& fsm, const GameActor& actor,
  const StateComponent& state, const Rigidbody& rb, const Animator& animator)
{
  uint32_t flags = CF_None;
  const InputState& btns = actor.input.normal;

  // Input directions (relative to opponent)
  if (HasState(btns, InputState::UP))    flags |= CF_InputUp;
  if (HasState(btns, InputState::DOWN))  flags |= CF_InputDown;

  if (state.onLeftSide)
  {
    if (HasState(btns, InputState::RIGHT)) flags |= CF_InputForward;
    if (HasState(btns, InputState::LEFT))  flags |= CF_InputBackward;
  }
  else
  {
    if (HasState(btns, InputState::LEFT))  flags |= CF_InputForward;
    if (HasState(btns, InputState::RIGHT)) flags |= CF_InputBackward;
  }

  // Buttons
  if (HasState(btns, InputState::BTN1)) flags |= CF_InputBtn1;
  if (HasState(btns, InputState::BTN2)) flags |= CF_InputBtn2;
  if (HasState(btns, InputState::BTN3)) flags |= CF_InputBtn3;
  if (HasState(btns, InputState::BTN4)) flags |= CF_InputBtn4;

  if (flags & (CF_InputBtn1 | CF_InputBtn2 | CF_InputBtn3))
    flags |= CF_AnyAttackBtn;

  // Special inputs (relative to opponent)
  const SpecialInputState& sp = actor.input.special;
  if (state.onLeftSide)
  {
    if (sp == SpecialInputState::QCF)   flags |= CF_SpecialQCF;
    if (sp == SpecialInputState::QCB)   flags |= CF_SpecialQCB;
    if (sp == SpecialInputState::DPF)   flags |= CF_SpecialDPF;
    if (sp == SpecialInputState::DPB)   flags |= CF_SpecialDPB;
    if (sp == SpecialInputState::RDash) flags |= CF_SpecialFDash;
    if (sp == SpecialInputState::LDash) flags |= CF_SpecialBDash;
  }
  else
  {
    if (sp == SpecialInputState::QCF)   flags |= CF_SpecialQCB;
    if (sp == SpecialInputState::QCB)   flags |= CF_SpecialQCF;
    if (sp == SpecialInputState::DPF)   flags |= CF_SpecialDPB;
    if (sp == SpecialInputState::DPB)   flags |= CF_SpecialDPF;
    if (sp == SpecialInputState::RDash) flags |= CF_SpecialBDash;
    if (sp == SpecialInputState::LDash) flags |= CF_SpecialFDash;
  }

  // Physics
  if (rb.IsGrounded()) flags |= CF_IsGrounded;
  else                 flags |= CF_IsAirborne;

  // Combat
  if (state.hitThisFrame)    flags |= CF_HitThisFrame;
  if (state.thrownThisFrame) flags |= CF_ThrownThisFrame;
  if (fsm.hitConfirmed)      flags |= CF_Hitting;

  // Completion
  const auto& stateDef = fsm.GetCurrentStateDef();
  if (stateDef.completionType == StateDefinition::Animation)
  {
    Animation* animData = GAnimArchive.GetAnimationData(animator.animCollectionID, animator.currentAnimationName);
    if (animData && !animator.looping && animator.frame >= animData->GetFrameCount() - 1)
      flags |= CF_AnimComplete;
  }
  if (stateDef.completionType == StateDefinition::Timer)
  {
    if (fsm.stateTotalFrames > 0 && fsm.stateFrame >= fsm.stateTotalFrames - 1)
      flags |= CF_TimerComplete;
  }

  // State flags
  if (actor.newInputs || actor.forceNewInputOnNextFrame)
    flags |= CF_NewInputs;

  return flags;
}

//______________________________________________________________________________
FighterStateID FighterFSMSystem::EvaluateTransitions(
  EntityID entity, FighterFSMComponent& fsm, uint32_t flags,
  const GameActor& actor, StateComponent& state, const Rigidbody& rb)
{
  const auto& stateDef = fsm.GetCurrentStateDef();

  // Check transition rules (sorted by descending priority, first match wins)
  for (const auto& rule : stateDef.transitions)
  {
    if ((flags & rule.requiredFlags) != rule.requiredFlags)
      continue;
    if (rule.forbiddenFlags && (flags & rule.forbiddenFlags))
      continue;

    // Hit resolver sentinel
    if (rule.targetState == FighterStateID::COUNT)
      return ResolveHitTarget(fsm, actor, state, rb);

    return rule.targetState;
  }

  // Normal cancel via AttackLinkMap (target combo)
  if ((stateDef.cancelFlags & StateDefinition::Cancel_Normal) && (flags & CF_Hitting))
  {
    if (ComponentArray<AttackLinkMap>::Get().HasComponent(entity))
    {
      AttackLinkMap& linkMap = ComponentArray<AttackLinkMap>::Get().GetComponent(entity);
      auto it = linkMap.links.find(state.actionState);
      if (it != linkMap.links.end() && HasState(actor.input.normal, it->second))
      {
        // Determine the target attack state from the button
        InputState cancelBtn = it->second;
        bool crouching = (state.stanceState == StanceState::CROUCHING);

        if (cancelBtn == InputState::BTN1)
          return crouching ? FighterStateID::CrouchingLight : FighterStateID::StandingLight;
        if (cancelBtn == InputState::BTN2)
          return crouching ? FighterStateID::CrouchingMedium : FighterStateID::StandingMedium;
        if (cancelBtn == InputState::BTN3)
          return crouching ? FighterStateID::CrouchingHeavy : FighterStateID::StandingHeavy;
      }
    }
  }

  // If no transition matched but completion condition is met, use completionTarget
  if ((flags & CF_AnimComplete) && stateDef.completionType == StateDefinition::Animation)
    return stateDef.completionTarget;
  if ((flags & CF_TimerComplete) && stateDef.completionType == StateDefinition::Timer)
    return stateDef.completionTarget;

  return fsm.currentState;
}

//______________________________________________________________________________
FighterStateID FighterFSMSystem::ResolveHitTarget(
  const FighterFSMComponent& fsm, const GameActor& actor,
  StateComponent& state, const Rigidbody& rb)
{
  if (state.thrownThisFrame)
  {
    state.thrownThisFrame = false;
    return FighterStateID::Grappled;
  }

  // Consume the hit flag
  state.hitThisFrame = false;

  const auto& stateDef = fsm.GetCurrentStateDef();
  const HitData& hitData = state.hitData;
  const HitType recvHitType = hitData.type;

  // Blocking check
  bool canBlockLow  = (recvHitType == HitType::Low  && state.stanceState == StanceState::CROUCHING);
  bool canBlockMid  = (recvHitType == HitType::Mid  && (state.stanceState == StanceState::CROUCHING || state.stanceState == StanceState::STANDING));
  bool canBlockHigh = (recvHitType == HitType::High && state.stanceState == StanceState::STANDING);

  if (stateDef.canBlock && rb.IsGrounded() && (canBlockLow || canBlockMid || canBlockHigh))
  {
    bool holdingBack = HasState(actor.input.normal, InputState::LEFT) && state.onLeftSide;
    holdingBack = holdingBack || (HasState(actor.input.normal, InputState::RIGHT) && !state.onLeftSide);
    if (holdingBack)
    {
      return state.stanceState == StanceState::CROUCHING
        ? FighterStateID::BlockstunCrouching
        : FighterStateID::BlockstunStanding;
    }
  }

  // Knockdown check
  if (stateDef.inKnockdown || hitData.knockdown)
    return FighterStateID::KnockdownAirborne;

  // Default hitstun
  return state.stanceState == StanceState::CROUCHING
    ? FighterStateID::CrouchingHitstun
    : FighterStateID::Hitstun;
}

//______________________________________________________________________________
void FighterFSMSystem::EnactState(
  EntityID entity, FighterFSMComponent& fsm, GameActor& actor,
  StateComponent& state, Rigidbody& rb, Animator& animator)
{
  const auto& stateDef = fsm.GetCurrentStateDef();

  // Update StateComponent
  state.stanceState = stateDef.stanceState;
  state.actionState = stateDef.actionState;
  state.onNewState = true;
  state.hitting = false;
  fsm.hitConfirmed = false;

  // Force input re-evaluation next frame (replaces old ForceNewInputOnChange lifecycle)
  actor.forceNewInputOnNextFrame = true;

  // --- Determine animation to play ---
  const char* animName = stateDef.animationName;
  float playSpeed = stateDef.playSpeed;

  // Hitstun: choose animation based on stun duration
  if (fsm.currentState == FighterStateID::Hitstun)
  {
    animName = "LightHitstun";
    if (state.hitData.framesInStunHit > 20) animName = "MedHitstun";
    if (state.hitData.framesInStunHit > 30) animName = "HeavyHitstun";
  }

  // Dash: adjust animation play speed to fit dash duration
  if (fsm.currentState == FighterStateID::ForwardDash || fsm.currentState == FighterStateID::BackDash)
  {
    Animation* dashAnim = GAnimArchive.GetAnimationData(animator.animCollectionID, animName);
    if (dashAnim)
    {
      int ssAnimFrames = dashAnim->GetFrameCount();
      playSpeed = static_cast<float>(ssAnimFrames) / static_cast<float>(GlobalVars::nDashFrames);
    }
  }

  // --- Play animation ---
  Animation* anim = animator.Play(animName, stateDef.loopAnimation, playSpeed, stateDef.forceAnimRestart);
  if (anim)
  {
    RenderProperties& properties = ComponentArray<RenderProperties>::Get().GetComponent(entity);
    RenderComponent<RenderType>& renderer = ComponentArray<RenderComponent<RenderType>>::Get().GetComponent(entity);
    properties.horizontalFlip = !state.onLeftSide;
    properties.anchor = anim->GetAnchorForAnimFrame(0).first;
    properties.offset = anim->GetAnchorForAnimFrame(0).second;
    properties.renderScaling = anim->GetRenderScaling();
    renderer.SetRenderResource(anim->GetSheetTexture<RenderType>());
    renderer.sourceRect = anim->GetFrameSrcRect(0);
  }

  // --- Entry movement ---
  switch (stateDef.entryMovement)
  {
    case StateDefinition::Stop:
      rb.velocity = Vector2<float>::Zero;
      break;
    case StateDefinition::StopHorizontal:
      rb.velocity.x = 0;
      break;
    case StateDefinition::UseHitKnockback:
      rb.velocity.x = state.hitData.knockback.x;
      break;
    case StateDefinition::UseHitKnockbackFull:
      rb.velocity = state.hitData.knockback;
      break;
    case StateDefinition::Custom:
      // Walk states
      if (fsm.currentState == FighterStateID::WalkForward)
      {
        float walkSpeed = 0.5f * GlobalVars::BaseWalkSpeed;
        rb.velocity.x = state.onLeftSide ? walkSpeed : -walkSpeed;
      }
      else if (fsm.currentState == FighterStateID::WalkBackward)
      {
        float walkSpeed = 0.5f * GlobalVars::BaseWalkSpeed;
        rb.velocity.x = state.onLeftSide ? -walkSpeed : walkSpeed;
      }
      // Jump state
      else if (fsm.currentState == FighterStateID::Jumping)
      {
        float jumpX = 0.0f;
        if (HasState(actor.input.normal, InputState::LEFT))
          jumpX = -0.5f * GlobalVars::BaseWalkSpeed;
        else if (HasState(actor.input.normal, InputState::RIGHT))
          jumpX = 0.5f * GlobalVars::BaseWalkSpeed;
        rb.velocity = Vector2<float>(jumpX, -GlobalVars::JumpVelocity);
        // Clear grounded flag so the landing transition doesn't fire
        // on the same frame — physics hasn't moved us off the ground yet
        rb.lastCollisionSide &= ~CollisionSide::DOWN;
      }
      break;
    case StateDefinition::NoMovement:
      break;
  }

  // --- Timer setup ---
  fsm.stateFrame = 0;
  fsm.statePlayTime = 0.0f;
  fsm.stateTotalFrames = 0;

  if (stateDef.completionType == StateDefinition::Timer)
  {
    int timerFrames = stateDef.timerFrames;
    // Dynamic timer values for damage states
    if (fsm.currentState == FighterStateID::Hitstun ||
        fsm.currentState == FighterStateID::CrouchingHitstun)
      timerFrames = state.hitData.framesInStunHit;
    else if (fsm.currentState == FighterStateID::BlockstunStanding ||
             fsm.currentState == FighterStateID::BlockstunCrouching)
      timerFrames = state.hitData.framesInStunBlock;
    else if (fsm.currentState == FighterStateID::Grappled)
      timerFrames = state.hitData.activeFrames;
    fsm.stateTotalFrames = timerFrames;
  }

  // --- Attack state component ---
  Entity* e = GameManager::Get().GetEntityByID(entity).get();
  if (stateDef.isAttackState)
  {
    if (GAnimArchive.GetCollection(animator.animCollectionID).GetEventList(animName))
    {
      e->AddComponent<AttackStateComponent>();
      e->GetComponent<AttackStateComponent>()->attackAnimation = animName;
    }
  }

  // --- Grapple state ---
  if (stateDef.isGrappleState)
  {
    rb.ignoreDynamicColliders = true;
    if (fsm.currentState == FighterStateID::ForwardThrow ||
        fsm.currentState == FighterStateID::BackThrow)
    {
      e->AddComponent<GrappleActionComponent>();
    }
  }

  // --- Grappled state (being thrown) ---
  if (fsm.currentState == FighterStateID::Grappled)
  {
    rb.ignoreDynamicColliders = true;
  }

  // --- Damage and SFX ---
  if (stateDef.appliesDamage)
  {
    bool fromGrapple = (fsm.previousState == FighterStateID::Grappled);

    if (stateDef.isBlocking)
    {
      // No HP loss on block
      if (GlobalVars::ShowHitEffects)
      {
        SFXComponent& sfx = ComponentArray<SFXComponent>::Get().GetComponent(entity);
        sfx.ShowBlockSparks(state.onLeftSide);
      }
      if (!fromGrapple)
        GameManager::Get().ActivateHitStop(GlobalVars::HitStopFramesOnBlock);
    }
    else
    {
      if (!state.invulnerable)
        state.hp -= state.hitData.damage;

      if (GlobalVars::ShowHitEffects && !fromGrapple)
      {
        SFXComponent& sfx = ComponentArray<SFXComponent>::Get().GetComponent(entity);
        sfx.ShowHitSparks(state.onLeftSide);
      }
      if (!fromGrapple)
        GameManager::Get().ActivateHitStop(GlobalVars::HitStopFramesOnHit);
    }
  }

  // --- Hit state component ---
  if (fsm.currentState == FighterStateID::Hitstun ||
      fsm.currentState == FighterStateID::CrouchingHitstun ||
      fsm.currentState == FighterStateID::BlockstunStanding ||
      fsm.currentState == FighterStateID::BlockstunCrouching ||
      fsm.currentState == FighterStateID::KnockdownAirborne)
  {
    e->AddComponent<HitStateComponent>();
  }

  // --- Juggle gravity ---
  if (stateDef.setsJuggleGravity)
    ComponentArray<Gravity>::Get().GetComponent(entity).force = GlobalVars::JuggleGravity;
  if (stateDef.resetsJuggleGravity)
    ComponentArray<Gravity>::Get().GetComponent(entity).force = GlobalVars::Gravity;
}

//______________________________________________________________________________
void FighterFSMSystem::UpdateCurrentState(
  float dt, EntityID entity, FighterFSMComponent& fsm,
  StateComponent& state, Rigidbody& rb)
{
  const auto& stateDef = fsm.GetCurrentStateDef();

  // Advance timer for timer-based states
  if (stateDef.completionType == StateDefinition::Timer && fsm.stateTotalFrames > 0)
  {
    fsm.statePlayTime += dt;
    int framesToAdv = static_cast<int>(std::floor(fsm.statePlayTime / secPerFrame));
    fsm.stateFrame += framesToAdv;
    fsm.statePlayTime -= (framesToAdv * secPerFrame);
  }

  // Dash velocity update using plateau interpolation
  if (fsm.currentState == FighterStateID::ForwardDash ||
      fsm.currentState == FighterStateID::BackDash)
  {
    bool forward = (fsm.currentState == FighterStateID::ForwardDash);
    float dashSpeed = GlobalVars::BaseWalkSpeed * 1.5f;
    // Determine sign based on side and direction
    if ((!state.onLeftSide && forward) || (state.onLeftSide && !forward))
      dashSpeed *= -1.0f;

    float t = static_cast<float>(fsm.stateFrame);
    float totalT = static_cast<float>(fsm.stateTotalFrames);
    rb.velocity = Vector2<float>(Interpolation::Plateau::F(t, totalT, dashSpeed), 0);
  }
}

//______________________________________________________________________________
void FighterFSMSystem::TransitionTo(
  EntityID entity, FighterFSMComponent& fsm, FighterStateID target,
  StateComponent& state, Rigidbody& rb)
{
  Entity* e = GameManager::Get().GetEntityByID(entity).get();
  const auto& oldStateDef = fsm.GetCurrentStateDef();

  // Clean up old state
  if (oldStateDef.isAttackState)
  {
    e->RemoveComponent<AttackStateComponent>();
    e->RemoveComponent<AttackActionComponent>();
  }

  if (oldStateDef.isGrappleState)
  {
    e->RemoveComponent<GrappleActionComponent>();
    rb.ignoreDynamicColliders = false;
  }

  // Remove hit state component if leaving a damage state
  bool wasInDamageState = (fsm.currentState == FighterStateID::Hitstun ||
                           fsm.currentState == FighterStateID::CrouchingHitstun ||
                           fsm.currentState == FighterStateID::BlockstunStanding ||
                           fsm.currentState == FighterStateID::BlockstunCrouching ||
                           fsm.currentState == FighterStateID::KnockdownAirborne);
  if (wasInDamageState)
    e->RemoveComponent<HitStateComponent>();

  if (fsm.currentState == FighterStateID::Grappled)
  {
    rb.ignoreDynamicColliders = false;
    e->RemoveComponent<ReceivedGrappleAction>();
  }

  // Remove wall push on any state change
  e->RemoveComponent<WallPushComponent>();

  // Set new state
  fsm.hitConfirmed = false;
  fsm.previousState = fsm.currentState;
  fsm.currentState = target;
  fsm.stateJustEntered = true;
  fsm.stateFrame = 0;
  fsm.statePlayTime = 0.0f;
  fsm.stateTotalFrames = 0;
}
