#include "Core/ECS/Entity.h"
#include "Core/ECS/EntityManager.h"

// for stupid scaling problem that still needs to be fixed
#include "Components/Transform.h"
#include "Components/Collider.h"
#include "Components/Hurtbox.h"
#include "Components/Rigidbody.h"
#include "Components/Hitbox.h"

#include "Managers/GameManagement.h"

#include <sstream>

//______________________________________________________________________________
Entity::Entity()
{
  _id = EntityManager::Get().RegisterEntity();
}

//______________________________________________________________________________
Entity::~Entity()
{
  // set signature to remove from all systems
  EntityManager::Get().SetSignature(_id, 0x0);
  CheckAgainstSystems(this);

  // destroy self
  EntityManager::Get().DestroyEntity(_id);
}

//______________________________________________________________________________
void Entity::Serialize(std::ostream& os) const
{
  //std::stringstream serializationLog;
  //serializationLog << "SERIALIZING: \n";

  const ComponentBitFlag& signature = GetSignature();
  // serialize bitset first to know which components are attached to this one
  os << signature;
  // loop through signature finding all attached components
  for (size_t compIndex = 0; compIndex < ECSGlobalStatus::NRegisteredComponents; compIndex++)
  {
    if (signature.test(compIndex))
    {
      //serializationLog << ECSCoordinator::Get().GetComponentName(compIndex) << "\n";
      // write component data to stream based on signature
      ECSCoordinator::Get().SerializeComponent(_id, os, compIndex);
    }
  }

  /*std::string log = serializationLog.str();
  std::cout << log;*/
}

//______________________________________________________________________________
void Entity::Deserialize(std::istream& is)
{
  //std::stringstream serializationLog;
  //serializationLog << "DESERIALIZING: \n";

  ComponentBitFlag signature;
  // stream first thing should be signature
  is >> signature;

  for (size_t compIndex = 0; compIndex < ECSGlobalStatus::NRegisteredComponents; compIndex++)
  {
    std::type_index compTypeIndex = ECSCoordinator::Get().GetTypeIndex(compIndex);
    if (signature.test(compIndex))
    {
      //serializationLog << ECSCoordinator::Get().GetComponentName(compIndex) << "\n";

      // if deleter is already present, component already attached to entity
      if (_deleteComponent.find(compTypeIndex) == _deleteComponent.end())
      {
        // add self via generator which needs to add deleter
        auto deleterFn = ECSCoordinator::Get().AddSelf(GetID(), compIndex);
        _deleteComponent.insert(std::make_pair(compTypeIndex, deleterFn));
      }
      // this should write data directly to component, so no need to do anything
      ECSCoordinator::Get().DeserializeComponent(GetID(), is, compIndex);
    }
    else
    {
      // only delete if its already present
      if (_deleteComponent.find(compTypeIndex) != _deleteComponent.end())
      {
        ECSCoordinator::Get().RemoveSelf(GetID(), compIndex);
        // be sure to remove deleter when removing components through the generator
        _deleteComponent.erase(compTypeIndex);
      }
    }
  }
  // set the new entity signature with all components registered
  EntityManager::Get().SetSignature(GetID(), signature);
  // recheck against systems to register with systems
  CheckAgainstSystems(this);

  //std::string log = serializationLog.str();
  //std::cout << log;
}

//______________________________________________________________________________
std::string Entity::Log()
{
  std::stringstream ss;
  ss << "Entity " << GetID() << "\n";

  ss << "ComponentBitSignature: ";
  const ComponentBitFlag& signature = GetSignature();
  ss << signature << "\n";

  for (size_t compIndex = 0; compIndex < ECSGlobalStatus::NRegisteredComponents; compIndex++)
  {
    if (signature.test(compIndex))
    {
      ss << ECSCoordinator::Get().LogData(_id, compIndex);
    }
  }
  ss << "\n";
  return ss.str();
}

//______________________________________________________________________________
SBuffer Entity::CreateEntitySnapshot() const
{
  std::stringstream stream;
  Serialize(stream);

  return SBuffer(std::istreambuf_iterator<char>(stream), {});
}

//______________________________________________________________________________
void Entity::LoadEntitySnapshot(const SBuffer& snapshot)
{
  std::stringstream stream;
  stream.write((const char*)snapshot.data(), snapshot.size());

  Deserialize(stream);
}

//______________________________________________________________________________
void Entity::RemoveAllComponents()
{
  for(auto func : _deleteComponent)
  {
    func.second();
  }
  CheckAgainstSystems(this);
}

//______________________________________________________________________________
void Entity::DestroySelf()
{
  GameManager::Get().TriggerEndOfFrame([this]() { GameManager::Get().DestroyEntity(shared_from_this()); });
}

//______________________________________________________________________________
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
  if (auto collider = GetComponent<Hitbox>())
  {
    collider->rect.Scale(transform.scale, scale);
  }
  
  transform.rect.Scale(transform.scale, scale);
  transform.scale.x = scale.x;
  transform.scale.y = scale.y;
}

//______________________________________________________________________________
void Entity::CheckAgainstSystems(Entity* entity)
{
  GameManager::Get().CheckAgainstSystems(entity);
}
