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

void Entity::ParseCommand(const std::string& command)
{
  auto split = StringUtils::Split(command, ' ');

  if (split.size() > 5)
    return;

  std::string component = split[0];
  std::string function = split[1];
  std::string parameter = split[2];
  std::string subparameter = split[3];
  std::string value = split[4];
  Transform& transform = *GetComponent<Transform>();

  if (component == "transform")
  {
    if (function == "set")
    {
      if (parameter == "scale")
      {

        //adjust side of collider so you dont fall through ground
        float newScaleValue = std::stof(value);

        if (subparameter == "x")
        {
          SetScale(Vector2<float>(newScaleValue, transform.scale.y));
        }
        if (subparameter == "y")
        {
          SetScale(Vector2<float>(transform.scale.x, newScaleValue));
        }
      }

      if (parameter == "rotation")
      {
        if (subparameter == "x")
        {
          transform.rotation.x = std::stof(value);
        }
        if (subparameter == "y")
        {
          transform.rotation.y = std::stof(value);
        }
      }
    }
  }
}

std::string Entity::GetIdentifier()
{
  std::string id = std::string("entity") + std::to_string(_creationId);
  return id;
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