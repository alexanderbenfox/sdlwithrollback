#pragma once
#include <vector>
#include <bitset>
#include <cstdint>
#include <initializer_list>

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
// Values are bit indices into ConditionFlags, not bitmasks.
enum ConditionFlag : size_t
{
  // Input directions (relative to opponent)
  CF_InputUp          = 0,
  CF_InputDown        = 1,
  CF_InputForward     = 2,
  CF_InputBackward    = 3,
  // Input buttons
  CF_InputBtn1        = 4,
  CF_InputBtn2        = 5,
  CF_InputBtn3        = 6,
  CF_InputBtn4        = 7,
  // Special inputs (relative to opponent)
  CF_SpecialQCF       = 8,
  CF_SpecialQCB       = 9,
  CF_SpecialDPF       = 10,
  CF_SpecialDPB       = 11,
  CF_SpecialFDash     = 12,
  CF_SpecialBDash     = 13,
  // Physics
  CF_IsGrounded       = 14,
  CF_IsAirborne       = 15,
  // Combat
  CF_HitThisFrame     = 16,
  CF_ThrownThisFrame  = 17,
  CF_Hitting          = 18,
  // Completion
  CF_AnimComplete     = 19,
  CF_TimerComplete    = 20,
  // State flags
  CF_NewInputs        = 21,
  // Any attack button
  CF_AnyAttackBtn     = 22,

  // Total built-in flags (user-defined flags start here)
  CF_BuiltInCount     = 23,
};

// 64 bits gives room for user-defined condition flags beyond the built-in set
constexpr size_t ConditionFlagCapacity = 64;
using ConditionFlags = std::bitset<ConditionFlagCapacity>;

// Build a ConditionFlags from a list of flag indices
inline ConditionFlags MakeFlags(std::initializer_list<ConditionFlag> flags)
{
  ConditionFlags result;
  for (auto f : flags) result.set(f);
  return result;
}

//______________________________________________________________________________
struct TransitionRule
{
  ConditionFlags requiredFlags;       // all must be true
  ConditionFlags forbiddenFlags;      // if ANY of these are set, rule does NOT fire
  FighterStateID targetState;         // COUNT = use hit resolver
  int8_t priority = 0;                // higher = evaluated first
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
