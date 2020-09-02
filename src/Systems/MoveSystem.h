#pragma once
#include "Core/ECS/ISystem.h"

#include "Components/Transform.h"
#include "Components/Camera.h"
#include "Components/Rigidbody.h"
#include "Components/Hurtbox.h"
#include "Components/Hitbox.h"
#include "Components/StateComponents/HitStateComponent.h"
#include "Components/Actors/GameActor.h"
#include "Components/MetaGameComponents.h"

#include "Components/ActionComponents.h"


struct WallMoveComponent : public IComponent
{
  bool leftWall;
};

class MoveWallSystem : public IMultiSystem<SysComponents<Camera>, SysComponents<WallMoveComponent, StaticCollider, Transform>>
{
public:
  static void DoTick(float dt)
  {
    for (const EntityID& entity : MainSystem::Registered)
    {
      Camera& camera = ComponentArray<Camera>::Get().GetComponent(entity);
      for(const EntityID& e2 : SubSystem::Registered)
      {
        WallMoveComponent& wm = ComponentArray<WallMoveComponent>::Get().GetComponent(e2);
        StaticCollider& collider = ComponentArray<StaticCollider>::Get().GetComponent(e2);
        Transform& transform = ComponentArray<Transform>::Get().GetComponent(e2);

        if(wm.leftWall)
        {
          transform.position = Vector2<float>(camera.rect.x - collider.rect.HalfWidth(), camera.rect.y + camera.rect.h / 2);
        }
        else
        {
          transform.position = Vector2<float>(camera.rect.x + camera.rect.w + collider.rect.HalfWidth(), camera.rect.y + camera.rect.h / 2);
        }
        collider.MoveToTransform(transform);
      }
    }
  }
};

class MoveSystemCamera : public ISystem<Transform, Camera>
{
public:
  static void DoTick(float dt)
  {
    for (const EntityID& entity : Registered)
    {
      Transform& transform = ComponentArray<Transform>::Get().GetComponent(entity);
      Camera& camera = ComponentArray<Camera>::Get().GetComponent(entity);

      camera.rect.x = static_cast<int>(std::floor(transform.position.x)) - camera.rect.w / 2;
      camera.rect.y = static_cast<int>(std::floor(transform.position.y)) - camera.rect.h / 2;

      // also update camera matrix here
      camera.matrix =
        Mat4::Translation(-transform.position.x, -transform.position.y, 0) *
        //Mat4::Translation(-origin.x, -origin.y, 0) *
        Mat4::Scale(camera.zoom, camera.zoom, 1.0f) *
        Mat4::RotationZAxis(transform.rotation.x) * // should be z here
        Mat4::Translation(camera.origin.x, camera.origin.y, 0);


      Vector2<float> worldPosition = transform.position;
      worldPosition.x /= (static_cast<float>(camera.rect.w) / 1.0f);
      worldPosition.y /= (static_cast<float>(camera.rect.h) / 1.0f);

      Vector2<float> worldOrigin = camera.origin;
      worldOrigin.x /= (static_cast<float>(camera.rect.w) / 1.0f);
      worldOrigin.y /= (static_cast<float>(camera.rect.h) / 1.0f);

      camera.worldMatrix =
        Mat4::Translation(worldPosition.x + camera.worldMatrixPosition.x, -worldPosition.y + camera.worldMatrixPosition.y, camera.worldMatrixPosition.z) *
        Mat4::Scale(camera.zoom, camera.zoom, 1.0f) *
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

    for (const EntityID& e1 : MainSystem::Registered)
    {
      int nTargets = 0;
      Vector2<float> aggregatePosition;
      for (const EntityID& e2 : SubSystem::Registered)
      {
        Transform& transform = ComponentArray<Transform>::Get().GetComponent(e2);
        aggregatePosition += transform.position;
        nTargets++;
      }

      Transform& transform = ComponentArray<Transform>::Get().GetComponent(e1);
      Camera& camera = ComponentArray<Camera>::Get().GetComponent(e1);

      // clamp camera position
      Vector2<float> lerpTarget = camera.clamp.Saturate(aggregatePosition / static_cast<float>(nTargets));
      Vector2<float> lerp = (lerpTarget - transform.position) * lerpFactor * dt;

      // apply the smooth movement to camera position
      transform.position += lerp;
    }
  }
};

class MoveSystemPhysCollider : public ISystem<Transform, DynamicCollider>
{
public:
  static void DoTick(float dt)
  {
    for (const EntityID& entity : Registered)
    {
      Transform& transform = ComponentArray<Transform>::Get().GetComponent(entity);
      DynamicCollider& rect = ComponentArray<DynamicCollider>::Get().GetComponent(entity);

      rect.MoveToTransform(transform);
      
    }
  }
};

class MoveSystemHurtbox : public ISystem<Transform, Hurtbox>
{
public:
  static void DoTick(float dt)
  {
    for (const EntityID& entity : Registered)
    {
      Transform& transform = ComponentArray<Transform>::Get().GetComponent(entity);
      Hurtbox& hurtbox = ComponentArray<Hurtbox>::Get().GetComponent(entity);

      hurtbox.MoveToTransform(transform);
      
    }
  }
};

class MoveSystemHitbox : public ISystem<Transform, Hitbox>
{
public:
  static void DoTick(float dt)
  {
    for (const EntityID& entity : Registered)
    {
      Transform& transform = ComponentArray<Transform>::Get().GetComponent(entity);
      Hitbox& hitbox = ComponentArray<Hitbox>::Get().GetComponent(entity);

      if (hitbox.travelWithTransform)
        hitbox.MoveToTransform(transform);
    }
  }
};

class MoveThrownEntitySystem : public IMultiSystem<SysComponents<ThrowFollower, TeamComponent>, SysComponents<Transform, Hurtbox, ReceivedGrappleAction>>
{
public:
  static void DoTick(float dt)
  {
    for (const EntityID& e1 : MainSystem::Registered)
    {
      ThrowFollower& throwbox = ComponentArray<ThrowFollower>::Get().GetComponent(e1);
      TeamComponent& throwerTeam = ComponentArray<TeamComponent>::Get().GetComponent(e1);

      for (const EntityID& e2 : SubSystem::Registered)
      {
        Transform& trans = ComponentArray<Transform>::Get().GetComponent(e2);
        Hurtbox& box = ComponentArray<Hurtbox>::Get().GetComponent(e2);

        auto location = throwbox.rect.GetCenter();
        Vector2<float> boxOffset(0, 0);
        trans.position = location - boxOffset;
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
