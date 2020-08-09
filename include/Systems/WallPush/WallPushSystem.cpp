#include "Systems/WallPush/WallPushSystem.h"
#include "GameManagement.h"

void WallPushSystem::DoTick(float dt)
{
  for (auto tuple : Tuples)
  {
    Rigidbody* rigidbody = std::get<Rigidbody*>(tuple.second);
    WallPushComponent* push = std::get<WallPushComponent*>(tuple.second);
    Transform* transform = std::get<Transform*>(tuple.second);

    rigidbody->_vel.x = push->velocity;
    push->amountPushed += push->velocity * dt;
    if (std::fabs(push->amountPushed) >= std::fabs(push->pushAmount))
    {
      rigidbody->_vel.x = 0;
      GameManager::Get().ScheduleTask([transform]() { transform->RemoveComponent<WallPushComponent >(); });
    }
  }

  // delete wall push component
  GameManager::Get().RunScheduledTasks();
}
