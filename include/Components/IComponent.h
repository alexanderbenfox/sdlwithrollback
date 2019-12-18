#pragma once
#include <memory>

#include "ResourceManager.h"

const float animation_fps = 24.0f;

class Entity;

// IDEA: Split up components into their data and functions that change that data
// Based on reducers in redux??


class IComponent
{
public:
  IComponent(std::shared_ptr<Entity> owner) : _owner(owner) {}
  //virtual ~IComponent() = 0;
  //template<typename... Args> virtual void Init(Args... params) {}
  //!
  virtual void Update(float dt) {}// = 0;
  //!
  virtual void OnFrameBegin() {}
  //!
  virtual void OnFrameEnd() {}
  //!
protected:
  std::shared_ptr<Entity> _owner;
  
};