#include "Entity.h"
#include "Components/Transform.h"
#include "Components/Collider.h"
#include "Components/Hurtbox.h"
#include "Components/Rigidbody.h"
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

  if (auto collider = GetComponent<RectColliderD>())
  {
    collider->rect.Scale(transform.scale, scale);
  }
  if (auto collider = GetComponent<Hurtbox>())
  {
    collider->rect.Scale(transform.scale, scale);
  }
  if (auto collider = GetComponent<DynamicCollider>())
  {
    collider->rect.Scale(transform.scale, scale);
  }
  if (auto collider = GetComponent<StaticCollider>())
  {
    collider->rect.Scale(transform.scale, scale);
  }
  
  transform.rect.Scale(transform.scale, scale);
  transform.scale.x = scale.x;
  transform.scale.y = scale.y;
}

void Entity::CheckAgainstSystems(Entity* entity)
{
  GameManager::Get().CheckAgainstSystems(entity);
}
