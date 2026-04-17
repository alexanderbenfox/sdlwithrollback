#pragma once
#include "Core/ECS/ISystem.h"
#include "Components/ActionComponents.h"
#include "Components/RenderComponent.h"
#include "Components/Hitbox.h"
#include "Components/StateComponents/AttackStateComponent.h"
#include "Components/StateComponents/HitStateComponent.h"
#include "AssetManagement/AnimationEvent.h"

//
// Lifecycle systems that replace OnAdd/OnRemove in action components.
// Each uses the Check() override pattern to detect entity membership changes.
//

// DashingAction, JumpingAction, CrouchingAction all set forceNewInputOnNextFrame
// on both add and remove. This template handles all three.
template <typename ActionComponent>
struct ForceNewInputOnChange : public ISystem<ActionComponent, GameActor>
{
  static void Check(Entity* entity)
  {
    auto& reg = ISystem<ActionComponent, GameActor>::Registered;
    bool wasMember = reg.count(entity->GetID());
    ISystem<ActionComponent, GameActor>::Check(entity);
    bool isMember = reg.count(entity->GetID());
    if (wasMember != isMember)
      ComponentArray<GameActor>::Get().GetComponent(entity->GetID()).forceNewInputOnNextFrame = true;
  }
};

using DashingActionLifecycle = ForceNewInputOnChange<DashingAction>;
using JumpingActionLifecycle = ForceNewInputOnChange<JumpingAction>;
using CrouchingActionLifecycle = ForceNewInputOnChange<CrouchingAction>;

// GrappleActionComponent and ReceivedGrappleAction both reset
// ignoreDynamicColliders on remove.
template <typename GrappleComponent>
struct ResetColliderOnGrappleRemove : public ISystem<GrappleComponent, Rigidbody>
{
  static void Check(Entity* entity)
  {
    auto& reg = ISystem<GrappleComponent, Rigidbody>::Registered;
    bool wasMember = reg.count(entity->GetID());
    ISystem<GrappleComponent, Rigidbody>::Check(entity);
    bool isMember = reg.count(entity->GetID());
    if (wasMember && !isMember)
    {
      if (ComponentArray<Rigidbody>::Get().HasComponent(entity->GetID()))
        ComponentArray<Rigidbody>::Get().GetComponent(entity->GetID()).ignoreDynamicColliders = false;
    }
  }
};

using GrappleActionLifecycle = ResetColliderOnGrappleRemove<GrappleActionComponent>;
using ReceivedGrappleLifecycle = ResetColliderOnGrappleRemove<ReceivedGrappleAction>;

// AttackStateComponent cleanup: end any in-progress animation events and reset display color.
// We call all End*Event functions unconditionally since they are safe no-ops when the
// corresponding component doesn't exist.
struct AttackStateLifecycle : public ISystem<AttackStateComponent>
{
  static void Check(Entity* entity)
  {
    bool wasMember = Registered.count(entity->GetID());
    ISystem::Check(entity);
    bool isMember = Registered.count(entity->GetID());
    if (wasMember && !isMember)
    {
      EntityID id = entity->GetID();
      AnimationEvent::EndHitboxEvent(id);
      AnimationEvent::EndThrowboxEvent(id);
      AnimationEvent::EndMovementEvent(id);
      AnimationEvent::EndEntitySpawnEvent(id);

      if (ComponentArray<RenderProperties>::Get().HasComponent(id))
        ComponentArray<RenderProperties>::Get().GetComponent(id).SetDisplayColor(255, 255, 255);
    }
  }
};

// HitStateComponent cleanup: reset display color on removal.
struct HitStateLifecycle : public ISystem<HitStateComponent>
{
  static void Check(Entity* entity)
  {
    bool wasMember = Registered.count(entity->GetID());
    ISystem::Check(entity);
    bool isMember = Registered.count(entity->GetID());
    if (wasMember && !isMember)
    {
      if (ComponentArray<RenderProperties>::Get().HasComponent(entity->GetID()))
        ComponentArray<RenderProperties>::Get().GetComponent(entity->GetID()).SetDisplayColor(255, 255, 255);
    }
  }
};

// Throwbox cleanup: reset throw state flags on removal.
struct ThrowboxLifecycle : public ISystem<Throwbox, StateComponent>
{
  static void Check(Entity* entity)
  {
    bool wasMember = Registered.count(entity->GetID());
    ISystem::Check(entity);
    bool isMember = Registered.count(entity->GetID());
    if (wasMember && !isMember)
    {
      if (ComponentArray<StateComponent>::Get().HasComponent(entity->GetID()))
      {
        StateComponent& state = ComponentArray<StateComponent>::Get().GetComponent(entity->GetID());
        state.triedToThrowThisFrame = false;
        state.throwSuccess = false;
      }
    }
  }
};
