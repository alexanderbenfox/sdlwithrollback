#pragma once
#include <memory>

#include "ResourceManager.h"

#include <iostream>
#include "Utils.h"

const float avg_animation_fps = 24.0f;
const float third_strike_fps = 16.0f;

const float animation_fps = third_strike_fps;

struct FrameData
{
  // Member initialization
  FrameData() = default;
  // # of start up frames, active frames, and recovery frames
  int startUp, active, recover;
  // # of frames the receiver should be stunned on hit or block after attacker returns to neutral (can be + or -)
  int onHitAdvantage, onBlockAdvantage;
  // damage that the move does
  int damage;
  // knockback vector oriented from attack's source
  Vector2<float> knockback;
  // number of frames of action pause
  int hitstop;
};

class Entity;

// IDEA: Split up components into their data and functions that change that data
// Based on reducers in redux??


class IComponent
{
public:
  IComponent(std::shared_ptr<Entity> owner) : _owner(owner) {}
  //!
  virtual ~IComponent() {}
  //!
  virtual void OnFrameBegin() {}
  //!
  virtual void OnFrameEnd() {}
  //!
  virtual void PreUpdate() {}
  //!
  virtual void PostUpdate() {}
  //!
  virtual void Draw() {}

  virtual bool ShareOwner(IComponent* component)
  {
    return component->_owner == _owner;
  }

protected:
  //! 
  std::shared_ptr<Entity> _owner;
  
};

template <typename T>
struct ComponentTraits
{
  static int ID;
  static const uint64_t GetSignature() { return 1LL << ID; }
};

template <class T>
int ComponentTraits<T>::ID = ConstComponentIDGenerator::NextID();
