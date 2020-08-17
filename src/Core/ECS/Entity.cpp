#include "Core/ECS/Entity.h"
#include "Core/ECS/EntityManager.h"

// for stupid scaling problem that still needs to be fixed
#include "Components/Transform.h"
#include "Components/Collider.h"
#include "Components/Hurtbox.h"
#include "Components/Rigidbody.h"

#include "Managers/GameManagement.h"

Entity::Entity()
{
  _id = EntityManager::Get().RegisterEntity();
}

Entity::~Entity()
{
  // set signature to remove from all systems
  EntityManager::Get().SetSignature(_id, 0x0);
  CheckAgainstSystems(this);

  // destroy self
  EntityManager::Get().DestroyEntity(_id);
}

Entity* Entity::Copy()
{
  Entity* nEntity = new Entity;

  // loop through signature finding all attached components
  const ComponentBitFlag& signature = GetSignature();
  for (size_t compIndex = 0; compIndex < ECSGlobalStatus::NRegisteredComponents; compIndex++)
  {
    std::type_index compTypeIndex = ComponentIDGenerator::GetTypeIndex(compIndex);
    if (signature.test(compIndex))
    {
      // if deleter is already present, component already attached to entity
      if (nEntity->_deleteComponent.find(compTypeIndex) == nEntity->_deleteComponent.end())
      {
        // add self via generator which needs to add deleter
        auto deleterFn = ComponentIDGenerator::AddSelf(nEntity->GetID(), compIndex);
        nEntity->_deleteComponent.insert(std::make_pair(compTypeIndex, deleterFn));
      }
      ComponentIDGenerator::CopyComponentData(GetID(), nEntity->GetID(), compIndex);
    }
    else
    {
      // only delete if its already present
      if (nEntity->_deleteComponent.find(compTypeIndex) != nEntity->_deleteComponent.end())
      {
        ComponentIDGenerator::RemoveSelf(nEntity->GetID(), compIndex);
        // be sure to remove deleter when removing components through the generator
        nEntity->_deleteComponent.erase(compTypeIndex);
      }
    }
  }

  // propogate entity creation and addition of new components to systems
  CheckAgainstSystems(nEntity);
  return nEntity;
}

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
