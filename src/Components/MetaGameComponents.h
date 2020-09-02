#pragma once
#include "Core/ECS/IComponent.h"

//! marks the entity as the loser of the round
struct LoserComponent : public IComponent {};

//! marks which team the entity is on (team A == player 1 and team B == player 2)
struct TeamComponent : public IComponent
{
  enum class Team
  {
    TeamA, TeamB
  };

  Team team = Team::TeamA;
  bool playerEntity = false;
};
