#pragma once
#include "Components/DebugComponent/DebugComponent.h"
#include "Core/Geometry2D/RectHelper.h"
#include "AssetManagement/StaticAssets/AnimationAssetData.h"

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

//! hitbox is the area that will hit the opponent
class StateComponent : public IDebugComponent
{
public:
  StateComponent() : IDebugComponent("State Component") {}

  //! assignment operators for no-copy
  StateComponent(const StateComponent& other);
  StateComponent(StateComponent&& other);
  StateComponent& operator=(const StateComponent& other);
  StateComponent& operator=(StateComponent&& other) noexcept;


  //! Allows different fields to be changed through the debug menu
  virtual void OnDebug() override;
  
  //! is player on left side of the other player
  bool onLeftSide;

  //! if there were collisions this frame, what were they
  CollisionSide collision;

  //! Defender hit state information
  bool hitThisFrame = false;
  bool thrownThisFrame = false;
  HitData hitData;
  int comboCounter = 0;

  //! Attacker state information
  bool hitting = false;
  bool throwSuccess = false;
  bool triedToThrowThisFrame = false;

  int hp = 100;
  bool invulnerable = false;

  ActionState actionState;
  StanceState stanceState;

  bool onNewState = false;

  //!
  bool operator==(const StateComponent& other) const
  {
    return collision == other.collision && onLeftSide == other.onLeftSide &&
      hitThisFrame == other.hitThisFrame && hitting == other.hitting && thrownThisFrame && other.thrownThisFrame &&
      actionState == other.actionState && stanceState == other.stanceState &&
      triedToThrowThisFrame == other.triedToThrowThisFrame && throwSuccess && other.throwSuccess;
  }

  bool operator!=(const StateComponent& other) const
  {
    return !(operator==(other));
  }

};

inline StateComponent::StateComponent(const StateComponent& other) : IDebugComponent()
{
  operator=(other);
}

inline StateComponent::StateComponent(StateComponent&& other) : IDebugComponent()
{
  operator=(other);
}

inline StateComponent& StateComponent::operator=(const StateComponent& other)
{
  IDebugComponent::operator=(other);
  this->onLeftSide = other.onLeftSide;
  this->collision = other.collision;
  this->hitThisFrame = other.hitThisFrame;
  this->thrownThisFrame = other.thrownThisFrame;
  this->hitData = other.hitData;
  this->hitting = other.hitting;
  this->hp = other.hp;
  this->actionState = other.actionState;
  this->stanceState = other.stanceState;
  this->throwSuccess = other.throwSuccess;
  this->triedToThrowThisFrame = other.triedToThrowThisFrame;
  return *this;
}

inline StateComponent& StateComponent::operator=(StateComponent&& other) noexcept
{
  IDebugComponent::operator=(other);
  this->onLeftSide = other.onLeftSide;
  this->collision = other.collision;
  this->hitThisFrame = other.hitThisFrame;
  this->thrownThisFrame = other.thrownThisFrame;
  this->hitData = other.hitData;
  this->hitting = other.hitting;
  this->hp = other.hp;
  this->actionState = other.actionState;
  this->stanceState = other.stanceState;
  this->throwSuccess = other.throwSuccess;
  this->triedToThrowThisFrame = other.triedToThrowThisFrame;
  return *this;
}
