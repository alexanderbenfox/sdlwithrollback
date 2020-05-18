#pragma once
#include "IComponent.h"

class Entity;

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

  void Erase(std::shared_ptr<T> component)
  {
    auto it = std::find(_components.begin(), _components.end(), component);
    if (it != _components.end())
    {
      _components.erase(it);
    }
  }

  void OnFrameBegin()
  {
    for(auto comp : _components)
      comp->OnFrameBegin();
  }

  void PreUpdate()
  {
    for (auto comp : _components)
      comp->PreUpdate();
  }

  void PostUpdate()
  {
    for (auto comp : _components)
      comp->PostUpdate();
  }

  void OnFrameEnd()
  {
    for (auto comp : _components)
      comp->OnFrameEnd();
  }

  void Draw()
  {
    for (auto comp : _components)
      comp->Draw();
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

