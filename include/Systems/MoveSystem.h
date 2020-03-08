#pragma once
#include "Systems/ISystem.h"

#include "Components/Transform.h"
#include "Components/Camera.h"
#include "Components/Collider.h"

class MoveSystemCamera : public ISystem<Transform, Camera>
{
public:
  static void DoTick(float dt)
  {
    for (auto tuple : Tuples)
    {
      Transform* transform = std::get<Transform*>(tuple.second);
      Camera* camera = std::get<Camera*>(tuple.second);

      camera->rect.x = static_cast<int>(std::floor(transform->position.x));
      camera->rect.y = static_cast<int>(std::floor(transform->position.y));
    }
  }
};

class MoveSystemRect : public ISystem<Transform, RectColliderD>
{
public:
  static void DoTick(float dt)
  {
    for (auto tuple : Tuples)
    {
      Transform* transform = std::get<Transform*>(tuple.second);
      RectColliderD* rect = std::get<RectColliderD*>(tuple.second);

      rect->MoveToTransform(*transform);
    }
  }
};

class MoveSystem : public ISystem<Transform>
{
public:
  static void DoTick(float dt)
  {
    MoveSystemCamera::DoTick(dt);
    MoveSystemRect::DoTick(dt);
  }
};
