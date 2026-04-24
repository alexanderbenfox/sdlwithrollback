#include "Core/FSM/StateEnumMaps.h"
#include <iostream>

// Helper macro for defining enum map entries
#define ENTRY(enumVal, str) { enumVal, str }

//______________________________________________________________________________
// FighterStateID
//______________________________________________________________________________
struct StateIDEntry { FighterStateID id; const char* name; };
static constexpr StateIDEntry kStateIDMap[] = {
  ENTRY(FighterStateID::Idle,               "Idle"),
  ENTRY(FighterStateID::WalkForward,        "WalkForward"),
  ENTRY(FighterStateID::WalkBackward,       "WalkBackward"),
  ENTRY(FighterStateID::Jumping,            "Jumping"),
  ENTRY(FighterStateID::Falling,            "Falling"),
  ENTRY(FighterStateID::CrouchTransition,   "CrouchTransition"),
  ENTRY(FighterStateID::Crouched,           "Crouched"),
  ENTRY(FighterStateID::ForwardDash,        "ForwardDash"),
  ENTRY(FighterStateID::BackDash,           "BackDash"),
  ENTRY(FighterStateID::StandingLight,      "StandingLight"),
  ENTRY(FighterStateID::StandingMedium,     "StandingMedium"),
  ENTRY(FighterStateID::StandingHeavy,      "StandingHeavy"),
  ENTRY(FighterStateID::CrouchingLight,     "CrouchingLight"),
  ENTRY(FighterStateID::CrouchingMedium,    "CrouchingMedium"),
  ENTRY(FighterStateID::CrouchingHeavy,     "CrouchingHeavy"),
  ENTRY(FighterStateID::JumpingLight,       "JumpingLight"),
  ENTRY(FighterStateID::JumpingMedium,      "JumpingMedium"),
  ENTRY(FighterStateID::JumpingHeavy,       "JumpingHeavy"),
  ENTRY(FighterStateID::SpecialMove1,       "SpecialMove1"),
  ENTRY(FighterStateID::SpecialMove2,       "SpecialMove2"),
  ENTRY(FighterStateID::SpecialMove3,       "SpecialMove3"),
  ENTRY(FighterStateID::SpecialMove4,       "SpecialMove4"),
  ENTRY(FighterStateID::ForwardThrow,       "ForwardThrow"),
  ENTRY(FighterStateID::BackThrow,          "BackThrow"),
  ENTRY(FighterStateID::ThrowMiss,          "ThrowMiss"),
  ENTRY(FighterStateID::Grappling,          "Grappling"),
  ENTRY(FighterStateID::Hitstun,            "Hitstun"),
  ENTRY(FighterStateID::CrouchingHitstun,   "CrouchingHitstun"),
  ENTRY(FighterStateID::BlockstunStanding,  "BlockstunStanding"),
  ENTRY(FighterStateID::BlockstunCrouching, "BlockstunCrouching"),
  ENTRY(FighterStateID::KnockdownAirborne,  "KnockdownAirborne"),
  ENTRY(FighterStateID::KnockdownHitGround, "KnockdownHitGround"),
  ENTRY(FighterStateID::KnockdownOnGround,  "KnockdownOnGround"),
  ENTRY(FighterStateID::Grappled,           "Grappled"),
};

const char* FighterStateIDToString(FighterStateID id)
{
  for (const auto& e : kStateIDMap)
    if (e.id == id) return e.name;
  return "Unknown";
}

FighterStateID FighterStateIDFromString(const std::string& str)
{
  if (str == "HitResolver") return FighterStateID::COUNT;
  for (const auto& e : kStateIDMap)
    if (str == e.name) return e.id;
  std::cerr << "Unknown FighterStateID: " << str << "\n";
  return FighterStateID::Idle;
}

//______________________________________________________________________________
// ConditionFlag
//______________________________________________________________________________
struct ConditionFlagEntry { ConditionFlag flag; const char* name; };
static constexpr ConditionFlagEntry kConditionFlagMap[] = {
  ENTRY(CF_InputUp,         "InputUp"),
  ENTRY(CF_InputDown,       "InputDown"),
  ENTRY(CF_InputForward,    "InputForward"),
  ENTRY(CF_InputBackward,   "InputBackward"),
  ENTRY(CF_InputBtn1,       "InputBtn1"),
  ENTRY(CF_InputBtn2,       "InputBtn2"),
  ENTRY(CF_InputBtn3,       "InputBtn3"),
  ENTRY(CF_InputBtn4,       "InputBtn4"),
  ENTRY(CF_SpecialQCF,      "SpecialQCF"),
  ENTRY(CF_SpecialQCB,      "SpecialQCB"),
  ENTRY(CF_SpecialDPF,      "SpecialDPF"),
  ENTRY(CF_SpecialDPB,      "SpecialDPB"),
  ENTRY(CF_SpecialFDash,    "SpecialFDash"),
  ENTRY(CF_SpecialBDash,    "SpecialBDash"),
  ENTRY(CF_IsGrounded,      "IsGrounded"),
  ENTRY(CF_IsAirborne,      "IsAirborne"),
  ENTRY(CF_HitThisFrame,    "HitThisFrame"),
  ENTRY(CF_ThrownThisFrame, "ThrownThisFrame"),
  ENTRY(CF_Hitting,         "Hitting"),
  ENTRY(CF_AnimComplete,    "AnimComplete"),
  ENTRY(CF_TimerComplete,   "TimerComplete"),
  ENTRY(CF_NewInputs,       "NewInputs"),
  ENTRY(CF_AnyAttackBtn,    "AnyAttackBtn"),
};

const char* ConditionFlagToString(ConditionFlag flag)
{
  for (const auto& e : kConditionFlagMap)
    if (e.flag == flag) return e.name;
  return "Unknown";
}

ConditionFlag ConditionFlagFromString(const std::string& str)
{
  for (const auto& e : kConditionFlagMap)
    if (str == e.name) return e.flag;
  std::cerr << "Unknown ConditionFlag: " << str << "\n";
  return CF_InputUp;
}

//______________________________________________________________________________
// StanceState
//______________________________________________________________________________
struct StanceStateEntry { StanceState state; const char* name; };
static constexpr StanceStateEntry kStanceStateMap[] = {
  ENTRY(StanceState::STANDING,  "Standing"),
  ENTRY(StanceState::CROUCHING, "Crouching"),
  ENTRY(StanceState::JUMPING,   "Jumping"),
  ENTRY(StanceState::KNOCKDOWN, "Knockdown"),
};

const char* StanceStateToString(StanceState s)
{
  for (const auto& e : kStanceStateMap)
    if (e.state == s) return e.name;
  return "Unknown";
}

StanceState StanceStateFromString(const std::string& str)
{
  for (const auto& e : kStanceStateMap)
    if (str == e.name) return e.state;
  std::cerr << "Unknown StanceState: " << str << "\n";
  return StanceState::STANDING;
}

//______________________________________________________________________________
// ActionState
//______________________________________________________________________________
struct ActionStateEntry { ActionState state; const char* name; };
static constexpr ActionStateEntry kActionStateMap[] = {
  ENTRY(ActionState::NONE,     "None"),
  ENTRY(ActionState::BLOCKSTUN,"Blockstun"),
  ENTRY(ActionState::HITSTUN,  "Hitstun"),
  ENTRY(ActionState::DASHING,  "Dashing"),
  ENTRY(ActionState::LIGHT,    "Light"),
  ENTRY(ActionState::MEDIUM,   "Medium"),
  ENTRY(ActionState::HEAVY,    "Heavy"),
};

const char* ActionStateToString(ActionState s)
{
  for (const auto& e : kActionStateMap)
    if (e.state == s) return e.name;
  return "Unknown";
}

ActionState ActionStateFromString(const std::string& str)
{
  for (const auto& e : kActionStateMap)
    if (str == e.name) return e.state;
  std::cerr << "Unknown ActionState: " << str << "\n";
  return ActionState::NONE;
}

//______________________________________________________________________________
// CompletionType
//______________________________________________________________________________
struct CompletionTypeEntry { StateDefinition::CompletionType type; const char* name; };
static constexpr CompletionTypeEntry kCompletionTypeMap[] = {
  ENTRY(StateDefinition::None,      "None"),
  ENTRY(StateDefinition::Timer,     "Timer"),
  ENTRY(StateDefinition::Animation, "Animation"),
};

const char* CompletionTypeToString(StateDefinition::CompletionType t)
{
  for (const auto& e : kCompletionTypeMap)
    if (e.type == t) return e.name;
  return "Unknown";
}

StateDefinition::CompletionType CompletionTypeFromString(const std::string& str)
{
  for (const auto& e : kCompletionTypeMap)
    if (str == e.name) return e.type;
  std::cerr << "Unknown CompletionType: " << str << "\n";
  return StateDefinition::None;
}

//______________________________________________________________________________
// EntryMovement
//______________________________________________________________________________
struct EntryMovementEntry { StateDefinition::EntryMovement movement; const char* name; };
static constexpr EntryMovementEntry kEntryMovementMap[] = {
  ENTRY(StateDefinition::NoMovement,          "NoMovement"),
  ENTRY(StateDefinition::Stop,                "Stop"),
  ENTRY(StateDefinition::StopHorizontal,      "StopHorizontal"),
  ENTRY(StateDefinition::UseHitKnockback,     "UseHitKnockback"),
  ENTRY(StateDefinition::UseHitKnockbackFull, "UseHitKnockbackFull"),
  ENTRY(StateDefinition::Custom,              "Custom"),
};

const char* EntryMovementToString(StateDefinition::EntryMovement m)
{
  for (const auto& e : kEntryMovementMap)
    if (e.movement == m) return e.name;
  return "Unknown";
}

StateDefinition::EntryMovement EntryMovementFromString(const std::string& str)
{
  for (const auto& e : kEntryMovementMap)
    if (str == e.name) return e.movement;
  std::cerr << "Unknown EntryMovement: " << str << "\n";
  return StateDefinition::NoMovement;
}

//______________________________________________________________________________
// CancelType
//______________________________________________________________________________
struct CancelTypeEntry { CancelType type; const char* name; };
static constexpr CancelTypeEntry kCancelTypeMap[] = {
  ENTRY(CancelType::NotCancel, "None"),
  ENTRY(CancelType::Cancel,   "Cancel"),
};

const char* CancelTypeToString(CancelType t)
{
  for (const auto& e : kCancelTypeMap)
    if (e.type == t) return e.name;
  return "None";
}

CancelType CancelTypeFromString(const std::string& str)
{
  for (const auto& e : kCancelTypeMap)
    if (str == e.name) return e.type;
  // Backwards compatibility
  if (str == "true" || str == "Normal" || str == "Special") return CancelType::Cancel;
  return CancelType::NotCancel;
}

#undef ENTRY
