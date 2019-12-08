#pragma once
#include "IComponent.h"

template <typename T = IComponent>
class ComponentManager
{
public:
  static ComponentManager<T>& Get()
  {
    static ComponentManager<T> manager;
    return manager;
  }

  std::shared_ptr<T> Create(std::shared_ptr<Entity> owner)
  {
    _components.push_back(std::make_shared<T>(owner));
    return _components.back();
  }

  void Update(float dt)
  {
    for (auto comp : _components)
      comp->Update(dt);
  }

  std::vector<std::shared_ptr<T>>& All() { return _components; }

private:
  //
  std::vector<std::shared_ptr<T>> _components;
  //
  ComponentManager() {}
  ComponentManager(const ComponentManager&) = delete;
  ComponentManager<T> operator=(ComponentManager&) = delete;
};