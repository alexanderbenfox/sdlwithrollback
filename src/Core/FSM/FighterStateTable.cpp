#include "Core/FSM/FighterStateTable.h"
#include <algorithm>

using ID = FighterStateID;

//______________________________________________________________________________
const FighterStateTable& FighterStateTable::Get()
{
  static FighterStateTable instance;
  return instance;
}

//______________________________________________________________________________
const std::array<StateDefinition, static_cast<size_t>(FighterStateID::COUNT)>& FighterStateTable::GetTable(uint8_t characterID) const
{
  // For now all characters use the same table
  return _ryuStates;
}

//______________________________________________________________________________
FighterStateTable::FighterStateTable()
{
  BuildRyu();
}

//______________________________________________________________________________
void FighterStateTable::BuildRyu()
{
  // ---- Reusable transition rule sets ----

  // Hit/thrown detection — highest priority, applies to any hittable state
  // targetState = COUNT means "use ResolveHitTarget"
  const TransitionRule hitTransition       = { CF_HitThisFrame,    0, ID::COUNT, 100 };
  const TransitionRule thrownTransition    = { CF_ThrownThisFrame, 0, ID::COUNT, 100 };

  // Fall detection — applies to any grounded state that can become airborne
  const TransitionRule fallTransition      = { CF_IsAirborne, 0, ID::Falling, 90 };

  // Special move transitions (priority 70, above normals)
  const TransitionRule donkeyKick = { CF_SpecialQCF | CF_InputBtn3 | CF_AnyAttackBtn, 0, ID::SpecialMove3, 72 };
  const TransitionRule fireball   = { CF_SpecialQCF | CF_AnyAttackBtn,                0, ID::SpecialMove1, 71 };
  const TransitionRule tatsu      = { CF_SpecialQCB | CF_AnyAttackBtn,                0, ID::SpecialMove4, 71 };
  const TransitionRule dp         = { CF_SpecialDPF | CF_AnyAttackBtn,                0, ID::SpecialMove2, 71 };

  // Normal attack transitions (priority 60)
  const TransitionRule throwFwd   = { CF_IsGrounded | CF_InputBtn4, CF_InputBackward, ID::ForwardThrow, 65 };
  const TransitionRule throwBack  = { CF_IsGrounded | CF_InputBtn4 | CF_InputBackward, 0, ID::BackThrow, 66 };
  const TransitionRule sLight     = { CF_IsGrounded | CF_InputBtn1, 0, ID::StandingLight,  60 };
  const TransitionRule sMedium    = { CF_IsGrounded | CF_InputBtn2, 0, ID::StandingMedium, 60 };
  const TransitionRule sHeavy     = { CF_IsGrounded | CF_InputBtn3, 0, ID::StandingHeavy,  60 };
  const TransitionRule cLight     = { CF_IsGrounded | CF_InputBtn1, 0, ID::CrouchingLight,  60 };
  const TransitionRule cMedium    = { CF_IsGrounded | CF_InputBtn2, 0, ID::CrouchingMedium, 60 };
  const TransitionRule cHeavy     = { CF_IsGrounded | CF_InputBtn3, 0, ID::CrouchingHeavy,  60 };
  const TransitionRule jLight     = { CF_IsAirborne | CF_InputBtn1, 0, ID::JumpingLight,  60 };
  const TransitionRule jMedium    = { CF_IsAirborne | CF_InputBtn2, 0, ID::JumpingMedium, 60 };
  const TransitionRule jHeavy     = { CF_IsAirborne | CF_InputBtn3, 0, ID::JumpingHeavy,  60 };

  // Dash transitions (priority 55)
  const TransitionRule dashFwdBtn = { CF_IsGrounded | CF_InputForward | CF_InputBtn4,  0, ID::ForwardDash, 55 };
  const TransitionRule dashFwdSp  = { CF_IsGrounded | CF_SpecialFDash,                 0, ID::ForwardDash, 55 };
  const TransitionRule dashBkBtn  = { CF_IsGrounded | CF_InputBackward | CF_InputBtn4, 0, ID::BackDash,    55 };
  const TransitionRule dashBkSp   = { CF_IsGrounded | CF_SpecialBDash,                 0, ID::BackDash,    55 };

  // Jump (priority 50)
  const TransitionRule jump       = { CF_IsGrounded | CF_InputUp, 0, ID::Jumping, 50 };

  // Crouch (priority 40)
  const TransitionRule crouch     = { CF_IsGrounded | CF_InputDown, 0, ID::CrouchTransition, 40 };

  // Walk (priority 30) — no CF_NewInputs: held directions should keep working after state changes
  const TransitionRule walkFwd    = { CF_IsGrounded | CF_InputForward,  0, ID::WalkForward,  30 };
  const TransitionRule walkBk     = { CF_IsGrounded | CF_InputBackward, 0, ID::WalkBackward, 30 };

  // Return to neutral (priority 20) — no direction held
  const TransitionRule returnToNeutral = { CF_NewInputs | CF_IsGrounded, CF_InputForward | CF_InputBackward | CF_InputDown, ID::Idle, 20 };

  // Completion transitions
  const TransitionRule onTimerToIdle     = { CF_TimerComplete, 0, ID::Idle, 10 };
  const TransitionRule onTimerToCrouch   = { CF_TimerComplete | CF_IsGrounded | CF_InputDown, 0, ID::CrouchTransition, 11 };
  const TransitionRule onAnimToIdle      = { CF_AnimComplete, 0, ID::Idle, 10 };
  const TransitionRule onAnimToCrouch    = { CF_AnimComplete | CF_IsGrounded | CF_InputDown, 0, ID::CrouchTransition, 11 };
  // Variants that skip CrouchTransition (for states already crouching)
  const TransitionRule onTimerToCrouched = { CF_TimerComplete | CF_IsGrounded | CF_InputDown, 0, ID::Crouched, 11 };
  const TransitionRule onAnimToCrouched  = { CF_AnimComplete | CF_IsGrounded | CF_InputDown, 0, ID::Crouched, 11 };
  const TransitionRule onGrounded        = { CF_IsGrounded, 0, ID::KnockdownHitGround, 10 };

  // ---- State Definitions ----

  // Idle
  {
    auto& s = _ryuStates[static_cast<size_t>(ID::Idle)];
    s.id = ID::Idle;
    s.animationName = "Idle";
    s.loopAnimation = true;
    s.forceAnimRestart = false;
    s.completionType = StateDefinition::None;
    s.stanceState = StanceState::STANDING;
    s.actionState = ActionState::NONE;
    s.isHittable = true;
    s.canBlock = true;
    s.entryMovement = StateDefinition::StopHorizontal;
    s.transitions = {
      hitTransition, thrownTransition, fallTransition,
      donkeyKick, fireball, tatsu, dp,
      throwBack, throwFwd,
      sLight, sMedium, sHeavy,
      dashFwdBtn, dashFwdSp, dashBkBtn, dashBkSp,
      jump, crouch,
      walkFwd, walkBk
    };
  }

  // WalkForward
  {
    auto& s = _ryuStates[static_cast<size_t>(ID::WalkForward)];
    s.id = ID::WalkForward;
    s.animationName = "WalkF";
    s.loopAnimation = true;
    s.stanceState = StanceState::STANDING;
    s.isHittable = true;
    s.canBlock = true;
    s.entryMovement = StateDefinition::Custom; // velocity set by EnactState based on direction
    s.horizontalOnly = true;
    s.transitions = {
      hitTransition, thrownTransition, fallTransition,
      donkeyKick, fireball, tatsu, dp,
      throwBack, throwFwd,
      sLight, sMedium, sHeavy,
      dashFwdBtn, dashFwdSp, dashBkBtn, dashBkSp,
      jump, crouch,
      walkBk, returnToNeutral
    };
  }

  // WalkBackward
  {
    auto& s = _ryuStates[static_cast<size_t>(ID::WalkBackward)];
    s.id = ID::WalkBackward;
    s.animationName = "WalkB";
    s.loopAnimation = true;
    s.stanceState = StanceState::STANDING;
    s.isHittable = true;
    s.canBlock = true;
    s.entryMovement = StateDefinition::Custom;
    s.horizontalOnly = true;
    s.transitions = {
      hitTransition, thrownTransition, fallTransition,
      donkeyKick, fireball, tatsu, dp,
      throwBack, throwFwd,
      sLight, sMedium, sHeavy,
      dashFwdBtn, dashFwdSp, dashBkBtn, dashBkSp,
      jump, crouch,
      walkFwd, returnToNeutral
    };
  }

  // Jumping
  {
    auto& s = _ryuStates[static_cast<size_t>(ID::Jumping)];
    s.id = ID::Jumping;
    s.animationName = "Jumping";
    s.loopAnimation = false;
    s.forceAnimRestart = false;
    s.completionType = StateDefinition::None;
    s.stanceState = StanceState::JUMPING;
    s.isHittable = true;
    s.entryMovement = StateDefinition::Custom; // jump velocity set by EnactState
    // Transitions: land → Idle, air attacks, hit
    s.transitions = {
      hitTransition, thrownTransition,
      jLight, jMedium, jHeavy,
      { CF_IsGrounded, 0, ID::Idle, 10 } // land
    };
  }

  // Falling (walked off edge or pushed off)
  {
    auto& s = _ryuStates[static_cast<size_t>(ID::Falling)];
    s.id = ID::Falling;
    s.animationName = "Falling";
    s.loopAnimation = true;
    s.forceAnimRestart = true;
    s.stanceState = StanceState::JUMPING;
    s.isHittable = true;
    s.entryMovement = StateDefinition::NoMovement; // keep existing velocity, remove horizontal
    s.transitions = {
      hitTransition, thrownTransition,
      jLight, jMedium, jHeavy,
      { CF_IsGrounded, 0, ID::Idle, 10 }
    };
  }

  // CrouchTransition (entering crouch with animation)
  {
    auto& s = _ryuStates[static_cast<size_t>(ID::CrouchTransition)];
    s.id = ID::CrouchTransition;
    s.animationName = "Crouching";
    s.loopAnimation = false;
    s.completionType = StateDefinition::Animation;
    s.completionTarget = ID::Crouched;
    s.stanceState = StanceState::CROUCHING;
    s.isHittable = true;
    s.canBlock = true;
    s.entryMovement = StateDefinition::StopHorizontal;
    s.transitions = {
      hitTransition, thrownTransition,
      // Can attack while crouching
      donkeyKick, fireball, tatsu, dp,
      cLight, cMedium, cHeavy,
      // Release down → go to idle
      { CF_NewInputs | CF_IsGrounded, CF_InputDown, ID::Idle, 20 }
    };
  }

  // Crouched (holding crouch, looped animation)
  {
    auto& s = _ryuStates[static_cast<size_t>(ID::Crouched)];
    s.id = ID::Crouched;
    s.animationName = "Crouch";
    s.loopAnimation = true;
    s.forceAnimRestart = true;
    s.stanceState = StanceState::CROUCHING;
    s.isHittable = true;
    s.canBlock = true;
    s.entryMovement = StateDefinition::StopHorizontal;
    s.transitions = {
      hitTransition, thrownTransition,
      donkeyKick, fireball, tatsu, dp,
      cLight, cMedium, cHeavy,
      // Release down → idle
      { CF_NewInputs | CF_IsGrounded, CF_InputDown, ID::Idle, 20 }
    };
  }

  // ForwardDash
  {
    auto& s = _ryuStates[static_cast<size_t>(ID::ForwardDash)];
    s.id = ID::ForwardDash;
    s.animationName = "ForwardDash";
    s.loopAnimation = false;
    s.forceAnimRestart = true;
    s.completionType = StateDefinition::Timer;
    s.timerFrames = GlobalVars::nDashFrames;
    s.completionTarget = ID::Idle;
    s.stanceState = StanceState::STANDING;
    s.isHittable = true;
    s.entryMovement = StateDefinition::NoMovement; // dash velocity handled by UpdateCurrentState
    s.transitions = {
      hitTransition, thrownTransition,
      onTimerToCrouch, onTimerToIdle
    };
  }

  // BackDash
  {
    auto& s = _ryuStates[static_cast<size_t>(ID::BackDash)];
    s.id = ID::BackDash;
    s.animationName = "BackDash";
    s.loopAnimation = false;
    s.forceAnimRestart = true;
    s.completionType = StateDefinition::Timer;
    s.timerFrames = GlobalVars::nDashFrames;
    s.completionTarget = ID::Idle;
    s.stanceState = StanceState::STANDING;
    s.isHittable = true;
    s.entryMovement = StateDefinition::NoMovement;
    s.transitions = {
      hitTransition, thrownTransition,
      onTimerToCrouch, onTimerToIdle
    };
  }

  // ---- Normal Attacks (standing) ----
  auto buildGroundAttack = [&](ID id, const char* anim, ActionState actionSt)
  {
    auto& s = _ryuStates[static_cast<size_t>(id)];
    s.id = id;
    s.animationName = anim;
    s.loopAnimation = false;
    s.forceAnimRestart = true;
    s.completionType = StateDefinition::Animation;
    s.completionTarget = ID::Idle;
    s.stanceState = StanceState::STANDING;
    s.actionState = actionSt;
    s.isHittable = true;
    s.isAttackState = true;
    s.entryMovement = StateDefinition::StopHorizontal;
    s.cancelFlags = StateDefinition::Cancel_HitGround | StateDefinition::Cancel_Special | StateDefinition::Cancel_Normal;
    s.transitions = {
      hitTransition, thrownTransition,
      // Cancel on special (requires hitting)
      { CF_Hitting | CF_SpecialQCF | CF_InputBtn3 | CF_AnyAttackBtn, 0, ID::SpecialMove3, 72 },
      { CF_Hitting | CF_SpecialQCF | CF_AnyAttackBtn,                0, ID::SpecialMove1, 71 },
      { CF_Hitting | CF_SpecialQCB | CF_AnyAttackBtn,                0, ID::SpecialMove4, 71 },
      { CF_Hitting | CF_SpecialDPF | CF_AnyAttackBtn,                0, ID::SpecialMove2, 71 },
      // Normal cancel (target combo) handled by evaluator checking AttackLinkMap
      onAnimToCrouch, onAnimToIdle
    };
  };

  buildGroundAttack(ID::StandingLight,  "StandingLight",  ActionState::LIGHT);
  buildGroundAttack(ID::StandingMedium, "StandingMedium", ActionState::MEDIUM);
  buildGroundAttack(ID::StandingHeavy,  "StandingHeavy",  ActionState::HEAVY);

  // ---- Crouching Attacks ----
  auto buildCrouchAttack = [&](ID id, const char* anim, ActionState actionSt)
  {
    auto& s = _ryuStates[static_cast<size_t>(id)];
    s.id = id;
    s.animationName = anim;
    s.loopAnimation = false;
    s.forceAnimRestart = true;
    s.completionType = StateDefinition::Animation;
    s.completionTarget = ID::Crouched;
    s.stanceState = StanceState::CROUCHING;
    s.actionState = actionSt;
    s.isHittable = true;
    s.isAttackState = true;
    s.entryMovement = StateDefinition::StopHorizontal;
    s.cancelFlags = StateDefinition::Cancel_HitGround | StateDefinition::Cancel_Special | StateDefinition::Cancel_Normal;
    s.transitions = {
      hitTransition, thrownTransition,
      { CF_Hitting | CF_SpecialQCF | CF_InputBtn3 | CF_AnyAttackBtn, 0, ID::SpecialMove3, 72 },
      { CF_Hitting | CF_SpecialQCF | CF_AnyAttackBtn,                0, ID::SpecialMove1, 71 },
      { CF_Hitting | CF_SpecialQCB | CF_AnyAttackBtn,                0, ID::SpecialMove4, 71 },
      { CF_Hitting | CF_SpecialDPF | CF_AnyAttackBtn,                0, ID::SpecialMove2, 71 },
      onAnimToCrouched, onAnimToIdle  // already crouching → skip CrouchTransition
    };
  };

  buildCrouchAttack(ID::CrouchingLight,  "CrouchingLight",  ActionState::LIGHT);
  buildCrouchAttack(ID::CrouchingMedium, "CrouchingMedium", ActionState::MEDIUM);
  buildCrouchAttack(ID::CrouchingHeavy,  "CrouchingHeavy",  ActionState::HEAVY);

  // ---- Jumping Attacks ----
  auto buildAirAttack = [&](ID id, const char* anim, ActionState actionSt)
  {
    auto& s = _ryuStates[static_cast<size_t>(id)];
    s.id = id;
    s.animationName = anim;
    s.loopAnimation = false;
    s.forceAnimRestart = true;
    s.completionType = StateDefinition::Animation;
    s.completionTarget = ID::Idle;
    s.stanceState = StanceState::JUMPING;
    s.actionState = actionSt;
    s.isHittable = true;
    s.isAttackState = true;
    s.entryMovement = StateDefinition::NoMovement; // keep air momentum
    s.cancelFlags = StateDefinition::Cancel_HitGround | StateDefinition::Cancel_Special;
    s.transitions = {
      hitTransition, thrownTransition,
      // Cancel on special in air
      { CF_Hitting | CF_SpecialQCF | CF_InputBtn3 | CF_AnyAttackBtn, 0, ID::SpecialMove3, 72 },
      { CF_Hitting | CF_SpecialQCF | CF_AnyAttackBtn,                0, ID::SpecialMove1, 71 },
      { CF_Hitting | CF_SpecialQCB | CF_AnyAttackBtn,                0, ID::SpecialMove4, 71 },
      { CF_Hitting | CF_SpecialDPF | CF_AnyAttackBtn,                0, ID::SpecialMove2, 71 },
      // Landing cancels the attack
      { CF_IsGrounded, 0, ID::Idle, 50 },
      onAnimToIdle
    };
  };

  buildAirAttack(ID::JumpingLight,  "JumpingLight",  ActionState::LIGHT);
  buildAirAttack(ID::JumpingMedium, "JumpingMedium", ActionState::MEDIUM);
  buildAirAttack(ID::JumpingHeavy,  "JumpingHeavy",  ActionState::HEAVY);

  // ---- Special Moves ----
  auto buildSpecial = [&](ID id, const char* anim)
  {
    auto& s = _ryuStates[static_cast<size_t>(id)];
    s.id = id;
    s.animationName = anim;
    s.loopAnimation = false;
    s.forceAnimRestart = true;
    s.completionType = StateDefinition::Animation;
    s.completionTarget = ID::Idle;
    s.stanceState = StanceState::STANDING;
    s.actionState = ActionState::HEAVY;
    s.isHittable = true;
    s.isAttackState = true;
    s.entryMovement = StateDefinition::StopHorizontal;
    s.transitions = {
      hitTransition, thrownTransition,
      onAnimToCrouch, onAnimToIdle
    };
  };

  buildSpecial(ID::SpecialMove1, "SpecialMove1"); // fireball
  buildSpecial(ID::SpecialMove2, "SpecialMove2"); // dp
  buildSpecial(ID::SpecialMove3, "SpecialMove3"); // donkey kick
  buildSpecial(ID::SpecialMove4, "SpecialMove4"); // tatsu

  // ---- Throws ----
  auto buildThrow = [&](ID id, const char* anim)
  {
    auto& s = _ryuStates[static_cast<size_t>(id)];
    s.id = id;
    s.animationName = anim;
    s.loopAnimation = false;
    s.forceAnimRestart = true;
    s.completionType = StateDefinition::Animation;
    s.completionTarget = ID::Idle;
    s.stanceState = StanceState::STANDING;
    s.isHittable = true;
    s.isAttackState = true;
    s.isGrappleState = true;
    s.entryMovement = StateDefinition::StopHorizontal;
    s.transitions = {
      hitTransition, thrownTransition,
      onAnimToIdle
    };
  };

  buildThrow(ID::ForwardThrow, "ForwardThrow");
  buildThrow(ID::BackThrow, "BackThrow");

  // ThrowMiss (whiffed throw, locked in animation)
  {
    auto& s = _ryuStates[static_cast<size_t>(ID::ThrowMiss)];
    s.id = ID::ThrowMiss;
    s.animationName = "ThrowMiss";
    s.loopAnimation = false;
    s.forceAnimRestart = true;
    s.completionType = StateDefinition::Animation;
    s.completionTarget = ID::Idle;
    s.stanceState = StanceState::STANDING;
    s.isHittable = true;
    s.transitions = {
      hitTransition, thrownTransition,
      onAnimToIdle
    };
  }

  // Grappling (actively throwing the opponent — currently same as throw attack state)
  // This state is entered when grapple connects, handled by throw state + GrappleActionComponent

  // ---- Damage States ----

  // Hitstun (timer-based, dynamic values filled by EnactState)
  {
    auto& s = _ryuStates[static_cast<size_t>(ID::Hitstun)];
    s.id = ID::Hitstun;
    s.animationName = "LightHitstun"; // overridden by EnactState based on stun duration
    s.loopAnimation = false;
    s.forceAnimRestart = true;
    s.completionType = StateDefinition::Timer;
    s.timerFrames = 0; // set dynamically from hitData.framesInStunHit
    s.completionTarget = ID::Idle;
    s.stanceState = StanceState::STANDING;
    s.actionState = ActionState::HITSTUN;
    s.isHittable = true;
    s.inKnockdown = false;
    s.entryMovement = StateDefinition::UseHitKnockbackFull;
    s.appliesDamage = true;
    s.transitions = {
      hitTransition, thrownTransition,
      onTimerToCrouch, onTimerToIdle
    };
  }

  // CrouchingHitstun
  {
    auto& s = _ryuStates[static_cast<size_t>(ID::CrouchingHitstun)];
    s.id = ID::CrouchingHitstun;
    s.animationName = "CrouchingHitstun";
    s.loopAnimation = false;
    s.forceAnimRestart = true;
    s.completionType = StateDefinition::Timer;
    s.timerFrames = 0; // set dynamically
    s.completionTarget = ID::Idle;
    s.stanceState = StanceState::CROUCHING;
    s.actionState = ActionState::HITSTUN;
    s.isHittable = true;
    s.inKnockdown = false;
    s.entryMovement = StateDefinition::UseHitKnockbackFull;
    s.appliesDamage = true;
    s.transitions = {
      hitTransition, thrownTransition,
      onTimerToCrouched, onTimerToIdle  // already crouching → skip CrouchTransition
    };
  }

  // BlockstunStanding
  {
    auto& s = _ryuStates[static_cast<size_t>(ID::BlockstunStanding)];
    s.id = ID::BlockstunStanding;
    s.animationName = "BlockMid";
    s.loopAnimation = false;
    s.forceAnimRestart = true;
    s.completionType = StateDefinition::Timer;
    s.timerFrames = 0; // set dynamically from hitData.framesInStunBlock
    s.completionTarget = ID::Idle;
    s.stanceState = StanceState::STANDING;
    s.actionState = ActionState::BLOCKSTUN;
    s.isHittable = true;
    s.canBlock = true;
    s.entryMovement = StateDefinition::StopHorizontal;
    s.appliesDamage = true;
    s.isBlocking = true;
    s.transitions = {
      hitTransition, thrownTransition,
      onTimerToCrouch, onTimerToIdle
    };
  }

  // BlockstunCrouching
  {
    auto& s = _ryuStates[static_cast<size_t>(ID::BlockstunCrouching)];
    s.id = ID::BlockstunCrouching;
    s.animationName = "BlockLow";
    s.loopAnimation = false;
    s.forceAnimRestart = true;
    s.completionType = StateDefinition::Timer;
    s.timerFrames = 0; // set dynamically
    s.completionTarget = ID::Idle;
    s.stanceState = StanceState::CROUCHING;
    s.actionState = ActionState::BLOCKSTUN;
    s.isHittable = true;
    s.canBlock = true;
    s.entryMovement = StateDefinition::StopHorizontal;
    s.appliesDamage = true;
    s.isBlocking = true;
    s.transitions = {
      hitTransition, thrownTransition,
      onTimerToCrouched, onTimerToIdle  // already crouching → skip CrouchTransition
    };
  }

  // KnockdownAirborne (launched into the air)
  {
    auto& s = _ryuStates[static_cast<size_t>(ID::KnockdownAirborne)];
    s.id = ID::KnockdownAirborne;
    s.animationName = "Knockdown_Air";
    s.loopAnimation = false;
    s.forceAnimRestart = true;
    s.completionType = StateDefinition::None; // transitions on landing
    s.stanceState = StanceState::KNOCKDOWN;
    s.actionState = ActionState::HITSTUN;
    s.isHittable = true;
    s.inKnockdown = true;
    s.entryMovement = StateDefinition::UseHitKnockbackFull;
    s.appliesDamage = true;
    s.setsJuggleGravity = true;
    s.transitions = {
      hitTransition, thrownTransition,
      onGrounded // land → KnockdownHitGround
    };
  }

  // KnockdownHitGround (OTG period, can still be hit)
  {
    auto& s = _ryuStates[static_cast<size_t>(ID::KnockdownHitGround)];
    s.id = ID::KnockdownHitGround;
    s.animationName = "Knockdown_HitGround";
    s.loopAnimation = false;
    s.completionType = StateDefinition::Animation;
    s.completionTarget = ID::KnockdownOnGround;
    s.stanceState = StanceState::KNOCKDOWN;
    s.actionState = ActionState::HITSTUN;
    s.isHittable = true;
    s.inKnockdown = true;
    s.entryMovement = StateDefinition::StopHorizontal;
    s.transitions = {
      hitTransition, thrownTransition,
      onAnimToIdle // fallback shouldn't fire normally since completionTarget handles it
    };
  }

  // KnockdownOnGround (invincible getup)
  {
    auto& s = _ryuStates[static_cast<size_t>(ID::KnockdownOnGround)];
    s.id = ID::KnockdownOnGround;
    s.animationName = "Knockdown_OnGround";
    s.loopAnimation = false;
    s.completionType = StateDefinition::Animation;
    s.completionTarget = ID::Idle;
    s.stanceState = StanceState::KNOCKDOWN;
    s.actionState = ActionState::NONE;
    s.isHittable = false; // invincible during getup
    s.entryMovement = StateDefinition::StopHorizontal;
    s.resetsJuggleGravity = true;
    s.transitions = {
      onAnimToIdle
    };
  }

  // Grappled (receiving a throw)
  {
    auto& s = _ryuStates[static_cast<size_t>(ID::Grappled)];
    s.id = ID::Grappled;
    s.animationName = "HeavyHitstun";
    s.loopAnimation = false;
    s.forceAnimRestart = true;
    s.completionType = StateDefinition::Timer;
    s.timerFrames = 0; // set dynamically from hitData.activeFrames
    s.completionTarget = ID::KnockdownAirborne;
    s.stanceState = StanceState::KNOCKDOWN;
    s.isGrappleState = true;
    s.entryMovement = StateDefinition::NoMovement;
    s.transitions = {
      // No explicit timer transition — completionTarget handles timer → KnockdownAirborne
    };
  }

  // Grappling (actively performing throw — uses the throw animation, managed by throw state)
  // Currently throw attacker stays in their throw attack state (ForwardThrow/BackThrow)
  // Grappling state is used when throw misses and opponent escapes
  {
    auto& s = _ryuStates[static_cast<size_t>(ID::Grappling)];
    s.id = ID::Grappling;
    s.animationName = "ThrowMiss";
    s.loopAnimation = false;
    s.forceAnimRestart = true;
    s.completionType = StateDefinition::Animation;
    s.completionTarget = ID::Idle;
    s.stanceState = StanceState::STANDING;
    s.isHittable = true;
    s.transitions = {
      hitTransition, thrownTransition,
      onAnimToIdle
    };
  }

  // Sort all transition lists by descending priority so first-match-wins
  for (auto& state : _ryuStates)
  {
    std::sort(state.transitions.begin(), state.transitions.end(),
      [](const TransitionRule& a, const TransitionRule& b) { return a.priority > b.priority; });
  }
}
