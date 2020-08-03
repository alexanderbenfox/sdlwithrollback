#include "Entity.h"
#include "Components/Transform.h"
#include "Components/Collider.h"
#include "GameManagement.h"

void Entity::RemoveAllComponents()
{
  for(auto func : _deleteComponent)
  {
    func.second();
  }
  CheckAgainstSystems(this);
}

void Entity::DestroySelf()
{
  GameManager::Get().TriggerEndOfFrame([this]() { GameManager::Get().DestroyEntity(shared_from_this()); });
}

void Entity::SetScale(Vector2<float> scale)
{
  Transform& transform = *GetComponent<Transform>();

  for(auto& component : _components)
  {
    if(RectColliderD* collider = dynamic_cast<RectColliderD*>(component.second.get()))
    {
      collider->rect.Scale(transform.scale, scale);
    }
  }
  
  transform.rect.Scale(transform.scale, scale);
  transform.scale.x = scale.x;
  transform.scale.y = scale.y;
}

void Entity::CheckAgainstSystems(Entity* entity)
{
  GameManager::Get().CheckAgainstSystems(entity);
}
