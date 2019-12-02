#include "Entity.h"

void Entity::Update(float dt)
{
  for (auto& component : _components)
    component.second->Update(transform, dt);
}

void Entity::PushToRenderer()
{
  for (auto& component : _components)
    component.second->PushToRenderer(transform);
}
