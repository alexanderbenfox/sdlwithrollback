#pragma once
#include <memory>

#include "ResourceManager.h"

#include <iostream>


const float avg_animation_fps = 24.0f;
const float third_strike_fps = 16.0f;

const float animation_fps = third_strike_fps;

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
  virtual void Update(float dt) {}// = 0;
  //!
  virtual void OnFrameBegin() {}
  //!
  virtual void OnFrameEnd() {}
  //!
  virtual void PreUpdate() {}
  //!
  virtual void PostUpdate() {}

protected:
  //!
  std::shared_ptr<Entity> _owner;
  
};