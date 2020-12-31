#pragma once
#include "Components/DebugComponent/DebugComponent.h"
#include "Core/Geometry2D/RectHelper.h"
#include "Core/FightingGameTypes/HitData.h"

#include "Core/Interfaces/Serializable.h"
#include <sstream>

//! hitbox is the area that will hit the opponent
class StateComponent : public IDebugComponent, public ISerializable
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

  void Serialize(std::ostream& os) const override;
  void Deserialize(std::istream& is) override;
  std::string Log() override;



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
  // to force rvalue assignment, use std::move
  IDebugComponent::operator=(std::move(other));
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

inline void StateComponent::Serialize(std::ostream& os) const
{
  Serializer<bool>::Serialize(os, onLeftSide);
  Serializer<CollisionSide>::Serialize(os, collision);
  Serializer<bool>::Serialize(os, hitThisFrame);
  Serializer<bool>::Serialize(os, thrownThisFrame);
  hitData.Serialize(os);
  Serializer<int>::Serialize(os, comboCounter);
  Serializer<bool>::Serialize(os, hitting);
  Serializer<bool>::Serialize(os, throwSuccess);
  Serializer<bool>::Serialize(os, triedToThrowThisFrame);
  Serializer<int>::Serialize(os, hp);
  Serializer<bool>::Serialize(os, invulnerable);
  Serializer<ActionState>::Serialize(os, actionState);
  Serializer<StanceState>::Serialize(os, stanceState);
  Serializer<bool>::Serialize(os, onNewState);
}

inline void StateComponent::Deserialize(std::istream& is)
{
  Serializer<bool>::Deserialize(is, onLeftSide);
  Serializer<CollisionSide>::Deserialize(is, collision);
  Serializer<bool>::Deserialize(is, hitThisFrame);
  Serializer<bool>::Deserialize(is, thrownThisFrame);
  hitData.Deserialize(is);
  Serializer<int>::Deserialize(is, comboCounter);
  Serializer<bool>::Deserialize(is, hitting);
  Serializer<bool>::Deserialize(is, throwSuccess);
  Serializer<bool>::Deserialize(is, triedToThrowThisFrame);
  Serializer<int>::Deserialize(is, hp);
  Serializer<bool>::Deserialize(is, invulnerable);
  Serializer<ActionState>::Deserialize(is, actionState);
  Serializer<StanceState>::Deserialize(is, stanceState);
  Serializer<bool>::Deserialize(is, onNewState);
}

inline std::string StateComponent::Log()
{
  std::stringstream ss;
  ss << "StateComponent: \n";
  ss << "\tOn Left Side: " << onLeftSide << "\n";
  ss << "\tCollision Side: " << (int)collision << "\n";
  ss << "\tHit this frame: " << hitThisFrame << "\n";
  ss << "\tThrown this frame: " << thrownThisFrame << "\n";
  ss << hitData.Log();
  ss << "\tCombo counter: " << comboCounter << "\n";
  ss << "\tHitting: " << hitting << "\n";
  ss << "\tThrow success: " << throwSuccess << "\n";
  ss << "\tTried to throw this frame: " << triedToThrowThisFrame << "\n";
  ss << "\tHP: " << hp << "\n";
  ss << "\tInvulnerable: " << invulnerable << "\n";
  ss << "\tAction State: " << (int)actionState << "\n";
  ss << "\tStance State: " << (int)stanceState << "\n";
  ss << "\tOn New State: " << onNewState << "\n";
  return ss.str();
}
