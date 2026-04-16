#include "Components/Transform.h"
#include "Managers/GameManagement.h"

Transform::Transform() :
  position(Vector2<float>(0.0f, 0.0f)),
  scale(Vector2<float>(1.0f, 1.0f)),
  rotation(Vector2<float>(0.0f, 0.0f)),
  rect(), IComponent()
{
}

void Transform::SetWidthAndHeight(float width, float height)
{
  rect = Rect<float>(0.0f, 0.0f, width, height);
}

void Transform::Serialize(std::ostream& os) const
{
  os << position;
  os << rotation;
  os << scale;
}

void Transform::Deserialize(std::istream& is)
{
  is >> position;
  is >> rotation;
  is >> scale;
}

std::string Transform::Log()
{
  std::stringstream ss;
  ss << "Transform: \n";
  ss << "\tPosition: x=" << position.x << " y=" << position.y << "\n";
  ss << "\tRotation: x=" << rotation.x << " y=" << rotation.y << "\n";
  ss << "\tScale: x=" << scale.x << " y=" << scale.y << "\n";
  return ss.str();
}

