#include "GameState/BattleScene.h"
#include "Managers/GameManagement.h"

#include "Components/Camera.h"
#include "Components/Animator.h"

#include "Components/Rigidbody.h"
#include "Components/Collider.h"
#include "Components/Input.h"
#include "Components/RenderComponent.h"
#include "Components/SFXComponent.h"

#include "Components/Actors/GameActor.h"

// for wall push
#include "Components/ActionComponents.h"

#include "Systems/Physics.h"
#include "Systems/AnimationSystem.h"
#include "Systems/MoveSystem.h"
#include "Systems/InputSystem.h"
#include "Systems/HitSystem.h"
#include "Systems/TimerSystem/TimerSystem.h"
#include "Systems/CheckBattleEndSystem.h"
#include "Systems/UISystem.h"
#include "Systems/AISystem.h"
#include "Systems/WallPush/WallPushSystem.h"

#include "Systems/ActionSystems/EnactActionSystem.h"
#include "Systems/ActionSystems/ActionListenerSystem.h"
#include "Systems/ActionSystems/ActionHandleInputSystem.h"
#include "Core/Prefab/ActionFactory.h"

#include "Managers/AnimationCollectionManager.h"

#include "GameState/MatchScene.h"

#include "Core/Prefab/CharacterConstructor.h"

IScene* SceneHelper::CreateScene(SceneType type)
{
  switch(type)
  {
    case SceneType::START:
      return new StartScene;
    case SceneType::BATTLEMODE:
      return new BattleModeSelect;
    case SceneType::CSELECT:
      return new CharacterSelect;
    case SceneType::MATCH:
      return new MatchScene;
    case SceneType::RESULTS:
      return new ResultsScene;
    default: return new StartScene;
  }
}

BattleScene::~BattleScene()
{
  //for (int i = 0; i < _uiEntities.size(); i++)
  //  GameManager::Get().DestroyEntity(_uiEntities[i]);

  GameManager::Get().DestroyEntity(_p1UIAnchor);
  GameManager::Get().DestroyEntity(_p2UIAnchor);

  // we are moving into the after match cutscene, so only remove game state related components
  _p1->RemoveComponents<GameActor, StateComponent, Hurtbox, UIContainer, WallPushComponent, TimerContainer>();
  _p2->RemoveComponents<GameActor, StateComponent, Hurtbox, UIContainer, WallPushComponent, TimerContainer>();
}

void BattleScene::Init(std::shared_ptr<Entity> p1, std::shared_ptr<Entity> p2)
{
  _p1 = p1;
  _p2 = p2;

  InitCharacter(Vector2<int>(100, 0), _p1, true);
  InitCharacter(Vector2<int>(400, 0), _p2, false);

  //set player state to neutral
  ActionFactory::GoToNeutralAction(_p1->GetID(), _p1->GetComponent<StateComponent>());
  ActionFactory::GoToNeutralAction(_p2->GetID(), _p2->GetComponent<StateComponent>());
  _p1->AddComponent<InputListenerComponent>();
  _p2->AddComponent<InputListenerComponent>();
}

void BattleScene::Update(float deltaTime)
{
  // transition entities to neutral before enacting
  TransitionToNeutralSystem::DoTick(deltaTime);

  // check the hitboxes potentially just created
  HitSystem::DoTick(deltaTime);
  ThrowSystem::DoTick(deltaTime);
  WallPushSystem::DoTick(deltaTime);

  // update player side system here cause it forces new state
  PlayerSideSystem::DoTick(deltaTime);

  // update ui based on state right before inputs are collected
  UIPositionUpdateSystem::DoTick(deltaTime);
  UIContainerUpdateSystem::DoTick(deltaTime);

  // update based on state at start of frame
  UpdateAISystem::DoTick(deltaTime);

  ////++++ state machine section ++++////
  InputSystem::DoTick(deltaTime);

  // Check action state machine after all game context gets updated
  StateTransitionAggregate::DoTick(deltaTime);
  HandleUpdateAggregate::DoTick(deltaTime);

  // Enact new action states then clean them up
  EnactAggregate::DoTick(deltaTime);
  if(deltaTime > 0)
    CleanUpActionSystem::PostUpdate();
  ////++++ end state machine section ++++///
  
  // update timer systems after state has been chosen
  TimedActionSystem::DoTick(deltaTime);

  // update animation listener
  AnimationListenerSystem::DoTick(deltaTime);
  AnimationSystem::DoTick(deltaTime);
  // advance attack event schedules before checking hitboxes
  AttackAnimationSystem::DoTick(deltaTime);

  // resolve collisions
  ApplyGravitySystem::DoTick(deltaTime);
  PhysicsSystem::DoTick(deltaTime);
  // update the location of the colliders
  MoveSystem::DoTick(deltaTime);
  // move walls according to camera position
  MoveWallSystem::DoTick(deltaTime);

  ////++++ section for state dependent auxilliary info systems ++++////

// do stuff that requires up to date actions
  FrameAdvantageSystem::DoTick(deltaTime);

  // update AI timers, UI timers (all non-state timers)
  TimerSystem::DoTick(deltaTime);

  ////++++ end section for state dependent auxilliary info systems ++++////

  // check for battle complete and scene change
  CheckBattleEndSystem::DoTick(deltaTime);
}

BattleScene::StageBorders BattleScene::CreateStageBorders(const Rect<float>& stageRect, int screenWidth, int screenHeight)
{
  StageBorders stage;
  stage.clamp = Rect<float>(stageRect.beg.x + screenWidth / 2, 0, stageRect.end.x - screenWidth / 2, stageRect.end.y - screenHeight / 2);

  const float borderWidth = 200;
  const float borderHeight = 80;

  stage.borders[0] = GameManager::Get().CreateEntity<Transform, StaticCollider>();
  stage.borders[0]->GetComponent<Transform>()->position.x = (stageRect.beg.x + stageRect.end.x) / 2.0f;
  stage.borders[0]->GetComponent<Transform>()->position.y = stageRect.end.y;
  stage.borders[0]->GetComponent<StaticCollider>()->Init(Vector2<double>(stageRect.beg.x, stageRect.end.y - borderHeight / 2.0f), Vector2<double>(stageRect.end.x, stageRect.end.y + borderHeight / 2.0f));
  stage.borders[0]->GetComponent<StaticCollider>()->MoveToTransform(*stage.borders[0]->GetComponent<Transform>());

  stage.borders[1] = GameManager::Get().CreateEntity<Transform, StaticCollider, WallMoveComponent>();
  stage.borders[1]->GetComponent<Transform>()->position.x = stageRect.beg.x - 100;
  stage.borders[1]->GetComponent<Transform>()->position.y = (stageRect.beg.y + stageRect.end.y) / 2.0f;
  stage.borders[1]->GetComponent<StaticCollider>()->Init(Vector2<double>(-borderWidth, 0), Vector2<double>(0, stageRect.Height()));
  stage.borders[1]->GetComponent<StaticCollider>()->MoveToTransform(*stage.borders[1]->GetComponent<Transform>());
  stage.borders[1]->GetComponent<WallMoveComponent>()->leftWall = true;

  stage.borders[2] = GameManager::Get().CreateEntity<Transform, StaticCollider, WallMoveComponent>();
  stage.borders[2]->GetComponent<Transform>()->position.x = stageRect.end.x + 100.0f;
  stage.borders[2]->GetComponent<Transform>()->position.y = (stageRect.beg.y + stageRect.end.y) / 2.0f;
  stage.borders[2]->GetComponent<StaticCollider>()->Init(Vector2<double>(0, 0), Vector2<double>(borderWidth, stageRect.Height()));
  stage.borders[2]->GetComponent<StaticCollider>()->MoveToTransform(*stage.borders[2]->GetComponent<Transform>());
  stage.borders[2]->GetComponent<WallMoveComponent>()->leftWall = false;
  
  return stage;
}


void BattleScene::InitCharacter(Vector2<float> position, std::shared_ptr<Entity> player, bool isPlayer1)
{
  //! this needs to be in both the character constructor and here which is annoying and should be fixed
  const Vector2<float> healthBarOffset = { 30.0f, 20.0f };
  const Vector2<float> p2UIOffset(-200 - healthBarOffset.x * 2, 0);

  // set up non-ui components
  CharacterConstructor::InitSpatialComponents(player, player->GetComponent<SelectedCharacterComponent>()->characterIdentifier, position);
  if (isPlayer1)
    player->GetComponent<TeamComponent>()->team = TeamComponent::Team::TeamA;
  else
    player->GetComponent<TeamComponent>()->team = TeamComponent::Team::TeamB;

  // set up ui elements
  if (isPlayer1)
  {
    _p1UIAnchor = CharacterConstructor::InitUIComponents(player);
    _p1UIAnchor->GetComponent<UITransform>()->anchor = UIAnchor::TL;
  }
  else
  {
    _p2UIAnchor = CharacterConstructor::InitUIComponents(player);
    _p2UIAnchor->GetComponent<UITransform>()->position += p2UIOffset;
    _p2UIAnchor->GetComponent<UITransform>()->anchor = UIAnchor::TR;
  }

  // add this component for doing magic series
  if (!player->GetComponent<AttackLinkMap>())
  {
    player->AddComponent<AttackLinkMap>();

    auto magicSeriesMap = player->GetComponent<AttackLinkMap>();
    magicSeriesMap->links[ActionState::LIGHT] = InputState::BTN2;
    magicSeriesMap->links[ActionState::MEDIUM] = InputState::BTN3;
  }

  //! Janky loading
  player->AddComponent<SFXComponent>();

}
