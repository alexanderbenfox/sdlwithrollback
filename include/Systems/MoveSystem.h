#pragma once
#include "Systems/ISystem.h"

#include "Components/Transform.h"
#include "Components/Camera.h"
#include "Components/Rigidbody.h"
#include "Components/Hurtbox.h"
#include "Components/Hitbox.h"
#include "Components/StateComponents/HitStateComponent.h"


struct WallMoveComponent : public IComponent
{
  WallMoveComponent(std::shared_ptr<Entity> entity) : IComponent(entity) {}
  bool leftWall;
};

class MoveWallSystem : public IMultiSystem<SysComponents<Camera>, SysComponents<WallMoveComponent, StaticCollider, Transform>>
{
public:
  static void DoTick(float dt)
  {
    for (auto tuple : MainSystem::Tuples)
    {
      Camera* camera = std::get<Camera*>(tuple.second);
      for(auto subTuple : SubSystem::Tuples)
      {
        WallMoveComponent* wm = std::get<WallMoveComponent*>(subTuple.second);
        StaticCollider* collider = std::get<StaticCollider*>(subTuple.second);
        Transform* transform = std::get<Transform*>(subTuple.second);

        if(wm->leftWall)
        {
          transform->position = Vector2<float>(camera->rect.x - collider->rect.HalfWidth(), camera->rect.y + camera->rect.h / 2);
        }
        else
        {
          transform->position = Vector2<float>(camera->rect.x + camera->rect.w + collider->rect.HalfWidth(), camera->rect.y + camera->rect.h / 2);
        }
        collider->MoveToTransform(*transform);
      }
    }
  }
};

class MoveSystemCamera : public ISystem<Transform, Camera>
{
public:
  static void DoTick(float dt)
  {
    for (auto tuple : Tuples)
    {
      const float lerpFactor = 10.0f;

      Transform* transform = std::get<Transform*>(tuple.second);
      Camera* camera = std::get<Camera*>(tuple.second);

      if (camera->followPlayers)
      {
        Vector2<float> p1Pos = camera->player1->GetComponent<Transform>()->position;
        Vector2<float> p2Pos = camera->player2->GetComponent<Transform>()->position;

        // clamp camera position
        Vector2<float> lerpTarget = camera->clamp.Saturate((p1Pos + p2Pos) / 2.0f);
        Vector2<float> lerp = (lerpTarget - transform->position) * lerpFactor * dt;

        // apply the smooth movement
        transform->position += lerp;
      }

      camera->rect.x = static_cast<int>(std::floor(transform->position.x)) - camera->rect.w / 2;
      camera->rect.y = static_cast<int>(std::floor(transform->position.y)) - camera->rect.h / 2;

      // also update camera matrix here
      camera->matrix =
        Mat4::Translation(-transform->position.x, -transform->position.y, 0) *
        //Mat4::Translation(-origin.x, -origin.y, 0) *
        Mat4::Scale(camera->zoom, camera->zoom, 1.0f) *
        Mat4::RotationZAxis(transform->rotation.x) * // should be z here
        Mat4::Translation(camera->origin.x, camera->origin.y, 0);


      Vector2<float> worldPosition = transform->position;
      worldPosition.x /= (static_cast<float>(camera->rect.w) / 1.0f);
      worldPosition.y /= (static_cast<float>(camera->rect.h) / 1.0f);

      Vector2<float> worldOrigin = camera->origin;
      worldOrigin.x /= (static_cast<float>(camera->rect.w) / 1.0f);
      worldOrigin.y /= (static_cast<float>(camera->rect.h) / 1.0f);

      camera->worldMatrix =
        Mat4::Translation(worldPosition.x + camera->worldMatrixPosition.x, -worldPosition.y + camera->worldMatrixPosition.y, camera->worldMatrixPosition.z) *
        Mat4::Scale(camera->zoom, camera->zoom, 1.0f) *
        Mat4::Translation(-worldOrigin.x, worldOrigin.y, 0);
    }
  }
};

class MoveSystemPhysCollider : public ISystem<Transform, DynamicCollider>
{
public:
  static void DoTick(float dt)
  {
    for (auto tuple : Tuples)
    {
      Transform* transform = std::get<Transform*>(tuple.second);
      DynamicCollider* rect = std::get<DynamicCollider*>(tuple.second);

      rect->MoveToTransform(*transform);
      
    }
  }
};

class MoveSystemHurtbox : public ISystem<Transform, Hurtbox>
{
public:
  static void DoTick(float dt)
  {
    for (auto tuple : Tuples)
    {
      Transform* transform = std::get<Transform*>(tuple.second);
      Hurtbox* hurtbox = std::get<Hurtbox*>(tuple.second);

      hurtbox->MoveToTransform(*transform);
      
    }
  }
};

class MoveSystemHitbox : public ISystem<Transform, Hitbox>
{
public:
  static void DoTick(float dt)
  {
    for (auto tuple : Tuples)
    {
      Transform* transform = std::get<Transform*>(tuple.second);
      Hitbox* hitbox = std::get<Hitbox*>(tuple.second);

      if (hitbox->travelWithTransform)
        hitbox->MoveToTransform(*transform);
    }
  }
};

class MoveThrownEntitySystem : public IMultiSystem<SysComponents<ThrowFollower, TeamComponent>, SysComponents<Transform, Hurtbox, ThrownStateComponent>>
{
public:
  static void DoTick(float dt)
  {
    for (auto tuple : MainSystem::Tuples)
    {
      ThrowFollower* throwbox = std::get<ThrowFollower*>(tuple.second);
      TeamComponent* throwerTeam = std::get<TeamComponent*>(tuple.second);

      for (auto subTuple : SubSystem::Tuples)
      {
        Transform* trans = std::get<Transform*>(subTuple.second);
        Hurtbox* box = std::get<Hurtbox*>(subTuple.second);

        auto location = throwbox->rect.GetCenter();
        Vector2<float> boxOffset(0, 0);
        trans->position = location - boxOffset;
      }
    }
  }
};

class MoveSystem : public ISystem<>
{
public:
  static void DoTick(float dt)
  {
    MoveSystemCamera::DoTick(dt);
    MoveSystemPhysCollider::DoTick(dt);
    MoveSystemHurtbox::DoTick(dt);
    MoveSystemHitbox::DoTick(dt);
    MoveThrownEntitySystem::DoTick(dt);
  }
};
