#pragma once
#include "Core/ECS/IComponent.h"

struct SelectedCharacterComponent : public IComponent
{
  std::string characterIdentifier = "";
};

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

struct MatchMetaComponent : public IComponent
{
  int roundNo = 0;
  int scoreP1 = 0;
  int scoreP2 = 0;

};
