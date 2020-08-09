#pragma once
#include "Systems/ISystem.h"

#include "Components/Transform.h"
#include "Components/Camera.h"
#include "Components/Rigidbody.h"
#include "Components/Hurtbox.h"
#include "Components/Hitbox.h"
#include "Components/StateComponents/HitStateComponent.h"
#include "Components/Actors/GameActor.h"

#include "Components/ActionComponents.h"


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
      Transform* transform = std::get<Transform*>(tuple.second);
      Camera* camera = std::get<Camera*>(tuple.second);

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

//! Follows any 'acting' unit... want to use actor component for this but its messed up....
class CameraFollowPlayerSystem : public IMultiSystem<SysComponents<Transform, Camera, CameraFollowsPlayers>, SysComponents<Transform, Actor>>
{
public:
  static void DoTick(float dt)
  {
    const float lerpFactor = 10.0f;

    for (auto t1 : MainSystem::Tuples)
    {
      int nTargets = 0;
      Vector2<float> aggregatePosition;
      for (auto t2 : SubSystem::Tuples)
      {
        Transform* transform = std::get<Transform*>(t2.second);
        aggregatePosition += transform->position;
        nTargets++;
      }

      Transform* transform = std::get<Transform*>(t1.second);
      Camera* camera = std::get<Camera*>(t1.second);

      // clamp camera position
      Vector2<float> lerpTarget = camera->clamp.Saturate(aggregatePosition / static_cast<float>(nTargets));
      Vector2<float> lerp = (lerpTarget - transform->position) * lerpFactor * dt;

      // apply the smooth movement to camera position
      transform->position += lerp;
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

class MoveThrownEntitySystem : public IMultiSystem<SysComponents<ThrowFollower, TeamComponent>, SysComponents<Transform, Hurtbox, ReceivedGrappleAction>>
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
  static void Check(Entity* e)
  {
    MoveSystemPhysCollider::Check(e);
    MoveSystemHurtbox::Check(e);
    MoveSystemHitbox::Check(e);
    MoveThrownEntitySystem::Check(e);
    CameraFollowPlayerSystem::Check(e);
    MoveSystemCamera::Check(e);
  }

  static void DoTick(float dt)
  {
    MoveSystemPhysCollider::DoTick(dt);
    MoveSystemHurtbox::DoTick(dt);
    MoveSystemHitbox::DoTick(dt);
    MoveThrownEntitySystem::DoTick(dt);

    CameraFollowPlayerSystem::DoTick(dt);
    MoveSystemCamera::DoTick(dt);
  }
};
