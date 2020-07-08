#pragma once
#include "IComponent.h"
#include "Core/Geometry2D/RectHelper.h"
#include "DebugGUI/DebugItem.h"
#include "StateMachine/StateEnums.h"
#include "AssetManagement/StaticAssets/AnimationAssetData.h"

//! marks the entity as the loser of the round
struct LoserComponent : public IComponent
{
  LoserComponent(std::shared_ptr<Entity> owner) : IComponent(owner) {}
};

//! marks which team the entity is on (team A == player 1 and team B == player 2)
struct TeamComponent : public IComponent
{
  enum class Team
  {
    TeamA, TeamB
  };
  TeamComponent(std::shared_ptr<Entity> owner) : IComponent(owner) {}

  Team team = Team::TeamA;
  bool playerEntity = false;
};

//! hitbox is the area that will hit the opponent
class StateComponent : public IComponent, public DebugItem
{
public:
  StateComponent() : IComponent(nullptr), DebugItem() {}
  StateComponent(std::shared_ptr<Entity> owner) : IComponent(owner), DebugItem("State Component") {}

  //! assignment operators for no-copy
  StateComponent(const StateComponent& other);
  StateComponent(StateComponent&& other);
  StateComponent& operator=(const StateComponent& other);
  StateComponent& operator=(StateComponent&& other) noexcept;

  //! Adds loser component to entity
  void MarkLoser();
  //! Allows different fields to be changed through the debug menu
  virtual void OnDebug() override;
  
  //! is player on left side of the other player
  bool onLeftSide;

  //! if there were collisions this frame, what were they
  CollisionSide collision;

  //! Defender hit state information
  bool hitThisFrame = false;
  bool hitOnLeftSide = false;
  HitData hitData;

  //! Attacker state information
  bool hitting = false;

  int hp = 100;
  bool invulnerable = false;

  ActionState actionState;
  StanceState stanceState;

  bool onNewState = false;

  //!
  bool operator==(const StateComponent& other) const
  {
    return collision == other.collision && onLeftSide == other.onLeftSide && hitThisFrame == other.hitThisFrame && hitting == other.hitting && actionState == other.actionState && stanceState == other.stanceState;
  }

  bool operator!=(const StateComponent& other) const
  {
    return !(operator==(other));
  }

};

inline StateComponent::StateComponent(const StateComponent& other) : IComponent(nullptr), DebugItem()
{
  operator=(other);
}

inline StateComponent::StateComponent(StateComponent&& other) : IComponent(nullptr), DebugItem()
{
  operator=(other);
}

inline StateComponent& StateComponent::operator=(const StateComponent& other)
{
  DebugItem::operator=(other);
  this->onLeftSide = other.onLeftSide;
  this->collision = other.collision;
  this->hitThisFrame = other.hitThisFrame;
  this->hitOnLeftSide = other.hitOnLeftSide;
  this->hitData = other.hitData;
  this->hitting = other.hitting;
  this->hp = other.hp;
  this->actionState = other.actionState;
  this->stanceState = other.stanceState;
  return *this;
}

inline StateComponent& StateComponent::operator=(StateComponent&& other) noexcept
{
  DebugItem::operator=(other);
  this->onLeftSide = other.onLeftSide;
  this->collision = other.collision;
  this->hitThisFrame = other.hitThisFrame;
  this->hitOnLeftSide = other.hitOnLeftSide;
  this->hitData = other.hitData;
  this->hitting = other.hitting;
  this->hp = other.hp;
  this->actionState = other.actionState;
  this->stanceState = other.stanceState;
  return *this;
}
