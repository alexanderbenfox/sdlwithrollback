#pragma once
#include "Core/FSM/CharacterState.h"
#include <string>

// String-to-enum and enum-to-string conversions for all FSM-related enums.
// Used at load time only (JSON parsing, debug display). Not performance-critical.

const char* FighterStateIDToString(FighterStateID id);
FighterStateID FighterStateIDFromString(const std::string& str);

const char* ConditionFlagToString(ConditionFlag flag);
ConditionFlag ConditionFlagFromString(const std::string& str);

const char* StanceStateToString(StanceState s);
StanceState StanceStateFromString(const std::string& str);

const char* ActionStateToString(ActionState s);
ActionState ActionStateFromString(const std::string& str);

const char* CompletionTypeToString(StateDefinition::CompletionType t);
StateDefinition::CompletionType CompletionTypeFromString(const std::string& str);

const char* EntryMovementToString(StateDefinition::EntryMovement m);
StateDefinition::EntryMovement EntryMovementFromString(const std::string& str);

const char* CancelFlagToString(StateDefinition::CancelFlag f);
StateDefinition::CancelFlag CancelFlagFromString(const std::string& str);
