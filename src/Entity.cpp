#include "Entity.h"
#include "Components/Collider.h"

void Entity::Update(float dt)
{
  for (auto& component : _components)
    component.second->Update(dt);
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
  if (auto collider = GetComponent<RectColliderD>())
  {
    double addedWidth = collider->rect.Width() * (scale.x - transform.scale.x) / 2.0;
    double addedHeight = collider->rect.Height() * (scale.y - transform.scale.y) / 2.0;

    collider->rect = Rect<double>(transform.position.x - addedWidth, transform.position.y - addedHeight,
      transform.position.x + collider->rect.Width() + addedWidth, transform.position.y + collider->rect.Height() + addedHeight);
  }
  transform.scale.x = scale.x;
  transform.scale.y = scale.y;
}
