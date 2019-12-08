#pragma once
#include <memory>

class Entity;

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