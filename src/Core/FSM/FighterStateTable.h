#pragma once
#include "Core/FSM/CharacterState.h"
#include <array>

//______________________________________________________________________________
class FighterStateTable
{
public:
  static const FighterStateTable& Get();

  const std::array<StateDefinition, static_cast<size_t>(FighterStateID::COUNT)>& GetTable(uint8_t characterID) const;

private:
  FighterStateTable();

  // For now, one table (Ryu). Add more per character later.
  std::array<StateDefinition, static_cast<size_t>(FighterStateID::COUNT)> _ryuStates;

  void BuildRyu();
};
