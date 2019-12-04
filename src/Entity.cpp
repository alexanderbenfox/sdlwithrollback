#include "Entity.h"

void Entity::Update(float dt)
{
  for (auto& component : _components)
    component.second->Update(dt);
}
