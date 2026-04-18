#pragma once
#include <vector>
#include <cstdint>

#include "Globals.h"

//______________________________________________________________________________
enum class FighterStateID : uint8_t
{
  Idle = 0,
  WalkForward,
  WalkBackward,
  Jumping,
  Falling,
  CrouchTransition,
  Crouched,
  ForwardDash,
  BackDash,
  // Normal attacks
  StandingLight,
  StandingMedium,
  StandingHeavy,
  CrouchingLight,
  CrouchingMedium,
  CrouchingHeavy,
  JumpingLight,
  JumpingMedium,
  JumpingHeavy,
  // Special attacks
  SpecialMove1,
  SpecialMove2,
  SpecialMove3,
  SpecialMove4,
  // Throws
  ForwardThrow,
  BackThrow,
  ThrowMiss,
  Grappling,
  // Damage states
  Hitstun,
  CrouchingHitstun,
  BlockstunStanding,
  BlockstunCrouching,
  KnockdownAirborne,
  KnockdownHitGround,
  KnockdownOnGround,
  Grappled,
  COUNT
};

//______________________________________________________________________________
// Condition flags evaluated once per entity per frame.
// Direction-relative: InputForward/Backward and SpecialQCF/QCB are resolved
// from raw input + onLeftSide so transition rules are side-independent.
enum ConditionFlag : uint32_t
{
  CF_None             = 0,
  // Input directions (relative to opponent)
  CF_InputUp          = 1 << 0,
  CF_InputDown        = 1 << 1,
  CF_InputForward     = 1 << 2,
  CF_InputBackward    = 1 << 3,
  // Input buttons
  CF_InputBtn1        = 1 << 4,
  CF_InputBtn2        = 1 << 5,
  CF_InputBtn3        = 1 << 6,
  CF_InputBtn4        = 1 << 7,
  // Special inputs (relative to opponent)
  CF_SpecialQCF       = 1 << 8,
  CF_SpecialQCB       = 1 << 9,
  CF_SpecialDPF       = 1 << 10,
  CF_SpecialDPB       = 1 << 11,
  CF_SpecialFDash     = 1 << 12,
  CF_SpecialBDash     = 1 << 13,
  // Physics
  CF_IsGrounded       = 1 << 14,
  CF_IsAirborne       = 1 << 15,
  // Combat
  CF_HitThisFrame     = 1 << 16,
  CF_ThrownThisFrame  = 1 << 17,
  CF_Hitting          = 1 << 18,
  // Completion
  CF_AnimComplete     = 1 << 19,
  CF_TimerComplete    = 1 << 20,
  // State flags
  CF_NewInputs        = 1 << 21,
  // Any attack button
  CF_AnyAttackBtn     = 1 << 22,
};

//______________________________________________________________________________
struct TransitionRule
{
  uint32_t requiredFlags;       // AND mask: all must be true
  uint32_t forbiddenFlags = 0;  // if ANY of these are set, rule does NOT fire
  FighterStateID targetState;   // COUNT = use hit resolver
  int8_t priority = 0;          // higher = evaluated first
};

//______________________________________________________________________________
struct StateDefinition
{
  FighterStateID id;

  // Animation
  const char* animationName = "";
  bool loopAnimation = false;
  float playSpeed = 1.0f;
  bool forceAnimRestart = false;

  // Completion
  enum CompletionType : uint8_t { None, Timer, Animation };
  CompletionType completionType = None;
  int timerFrames = 0;
  FighterStateID completionTarget = FighterStateID::Idle;

  // State properties
  StanceState stanceState = StanceState::STANDING;
  ActionState actionState = ActionState::NONE;
  bool isHittable = false;
  bool canBlock = false;
  bool inKnockdown = false;
  bool isAttackState = false;
  bool isGrappleState = false;

  // Entry movement
  enum EntryMovement : uint8_t { NoMovement, Stop, StopHorizontal, UseHitKnockback, UseHitKnockbackFull, Custom };
  EntryMovement entryMovement = NoMovement;
  bool horizontalOnly = false;

  // Cancel types active in this state
  enum CancelFlag : uint8_t {
    Cancel_None      = 0,
    Cancel_HitGround = 1 << 0,
    Cancel_Special   = 1 << 1,
    Cancel_Normal    = 1 << 2,
  };
  uint8_t cancelFlags = Cancel_None;

  // Damage-related (for hit/block states)
  bool appliesDamage = false;
  bool isBlocking = false;
  bool setsJuggleGravity = false;
  bool resetsJuggleGravity = false;

  // Transitions (evaluated in priority order, first match wins)
  std::vector<TransitionRule> transitions;
};
