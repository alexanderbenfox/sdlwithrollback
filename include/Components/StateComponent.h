#pragma once
#include "IComponent.h"
#include "Core/Geometry2D/RectHelper.h"
#include "DebugGUI/DebugItem.h"

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
  
  //! is player on left side of the other player
  bool onLeftSide;

  //! if there were collisions this frame, what were they
  CollisionSide collision;

  //! Defender hit state information
  bool hitThisFrame = false;
  bool hitOnLeftSide = false;
  FrameData frameData;

  //! Attacker state information
  bool hitting = false;


  int hp = 100;

  virtual void OnDebug() override
  {
    //ImGui::Text("Player %d State Component", _owner->GetID());
    if(hitting)
      ImGui::Text("IsHitting");
    else
      ImGui::Text("NotHitting");
    ImGui::Text("HP = %d", hp);
  }

  //!
  bool operator==(const StateComponent& other) const
  {
    return collision == other.collision && onLeftSide == other.onLeftSide && hitThisFrame == other.hitThisFrame && hitting == other.hitting;
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
  this->frameData = other.frameData;
  this->hitting = other.hitting;
  return *this;
}

inline StateComponent& StateComponent::operator=(StateComponent&& other) noexcept
{
  DebugItem::operator=(other);
  this->onLeftSide = other.onLeftSide;
  this->collision = other.collision;
  this->hitThisFrame = other.hitThisFrame;
  this->hitOnLeftSide = other.hitOnLeftSide;
  this->frameData = other.frameData;
  this->hitting = other.hitting;
  return *this;
}
