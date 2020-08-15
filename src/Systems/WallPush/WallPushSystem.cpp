#include "Systems/WallPush/WallPushSystem.h"
#include "Managers/GameManagement.h"

void WallPushSystem::DoTick(float dt)
{
  DeferScopeGuard guard;
  for (const EntityID& entity : Registered)
  {
    Rigidbody& rigidbody = ComponentArray<Rigidbody>::Get().GetComponent(entity);
    WallPushComponent& push = ComponentArray<WallPushComponent>::Get().GetComponent(entity);
    Transform& transform = ComponentArray<Transform>::Get().GetComponent(entity);

    rigidbody._vel.x = push.velocity;
    push.amountPushed += push.velocity * dt;
    if (std::fabs(push.amountPushed) >= std::fabs(push.pushAmount))
    {
      rigidbody._vel.x = 0;
      defer(entity, GameManager::Get().GetEntityByID(entity)->RemoveComponent<WallPushComponent>());
    }
  }
}
