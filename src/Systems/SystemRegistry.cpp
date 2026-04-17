#include "Systems/SystemRegistry.h"

#include "Systems/Physics.h"
#include "Systems/AnimationSystem.h"
#include "Systems/DrawCallSystems.h"
#include "Systems/MoveSystem.h"
#include "Systems/InputSystem.h"
#include "Systems/HitSystem.h"
#include "Systems/TimerSystem/TimerSystem.h"
#include "Systems/CutsceneSystem.h"
#include "Systems/CheckBattleEndSystem.h"
#include "Systems/UISystem.h"
#include "Systems/AISystem.h"
#include "Systems/DestroyEntitiesSystem.h"
#include "Systems/MenuSystem.h"
#include "Systems/WallPush/WallPushSystem.h"

#include "Systems/ActionSystems/EnactActionSystem.h"
#include "Systems/ActionSystems/ActionListenerSystem.h"
#include "Systems/ActionSystems/ActionHandleInputSystem.h"

//______________________________________________________________________________
void CheckAgainstAllSystems(Entity* entity)
{
  InputSystem::Check(entity);
  ApplyGravitySystem::Check(entity);
  PhysicsSystem::Check(entity);
  AnimationSystem::Check(entity);
  MoveWallSystem::Check(entity);
  AttackAnimationSystem::Check(entity);
  HitSystem::Check(entity);
  TimerSystem::Check(entity);
  FrameAdvantageSystem::Check(entity);
  SpriteDrawCallSystem::Check(entity);
  UITextDrawCallSystem::Check(entity);
  PlayerSideSystem::Check(entity);
  CutsceneSystem::Check(entity);
  CutsceneMovementSystem::Check(entity);
  CheckBattleEndSystem::Check(entity);
  UIPositionUpdateSystem::Check(entity);
  UIContainerUpdateSystem::Check(entity);
  DrawUIPrimitivesSystem::Check(entity);
  DrawUIBoxSpriteSystem::Check(entity);
  UpdateAISystem::Check(entity);
  ThrowSystem::Check(entity);
  WallPushSystem::Check(entity);
  DestroyEntitiesSystem::Check(entity);
  MenuInputSystem::Check(entity);
  UpdateMenuStateSystem::Check(entity);

  AnimationListenerSystem::Check(entity);

  // aggregate systems
  StateTransitionAggregate::Check(entity);
  HandleUpdateAggregate::Check(entity);
  EnactAggregate::Check(entity);

  MoveSystem::Check(entity);
}
