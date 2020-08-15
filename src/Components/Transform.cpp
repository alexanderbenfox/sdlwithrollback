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

/*void Transform::AddChild(std::shared_ptr<Entity> child)
{
  children.push_back(child);
}

void Transform::RemoveChild(std::shared_ptr<Entity> child)
{
  children.erase(std::find(children.begin(), children.end(), child));
  GameManager::Get().DestroyEntity(child);
}

void Transform::RemoveAllChildren()
{
  for (const auto child : children)
    GameManager::Get().DestroyEntity(child);
  children.clear();
}*/

std::ostream& operator<<(std::ostream& os, const Transform& transform)
{
  os << transform.position;
  os << transform.scale;
  os << transform.rotation;
  return os;
}

std::istream& operator>>(std::istream& is, Transform& transform)
{
  is >> transform.position;
  is >> transform.rotation;
  is >> transform.scale;
  return is;
}