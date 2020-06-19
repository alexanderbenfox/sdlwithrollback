#include "GameState/Scene.h"
#include "GameManagement.h"

#include "Components/Camera.h"
#include "Components/Animator.h"
#include "Components/GameActor.h"
#include "Components/Rigidbody.h"
#include "Components/Collider.h"
#include "Components/Input.h"
#include "Components/RenderComponent.h"

#include "Systems/Physics.h"
#include "Systems/AnimationSystem.h"
#include "Systems/MoveSystem.h"
#include "Systems/InputSystem.h"
#include "Systems/HitSystem.h"
#include "Systems/TimerSystem.h"
#include "Systems/CheckBattleEndSystem.h"
#include "Systems/CutsceneSystem.h"

#include "AssetManagement/StaticAssets/CharacterConfig.h"

IScene* SceneHelper::CreateScene(SceneType type)
{
  switch(type)
  {
    case SceneType::START:
      return new StartScene;
    case SceneType::CSELECT:
      return new CharacterSelectScene;
    case SceneType::BATTLE:
      return new BattleScene;
    case SceneType::POSTMATCH:
      return new PostMatchScene;
    case SceneType::RESULTS:
      return new ResultsScene;
  }
}

BattleScene::~BattleScene()
{
  GameManager::Get().DestroyEntity(_borders[0]);
  GameManager::Get().DestroyEntity(_borders[1]);
  GameManager::Get().DestroyEntity(_borders[2]);
  GameManager::Get().DestroyEntity(_camera);

  // we are moving into the after match cutscene, so only remove game state related components
  _p1->RemoveComponents<GameActor, Hurtbox, StateComponent>();
  _p2->RemoveComponents<GameActor, Hurtbox, StateComponent>();
}

void BattleScene::Init(std::shared_ptr<Entity> p1, std::shared_ptr<Entity> p2)
{
  _p1 = p1;
  _p2 = p2;

  _borders[0] = GameManager::Get().CreateEntity<Transform, RenderComponent<RenderType>, StaticCollider>();
  _borders[0]->GetComponent<Transform>()->position.x = (float)m_nativeWidth / 2.0f;
  _borders[0]->GetComponent<Transform>()->position.y = m_nativeHeight;
  _borders[0]->GetComponent<RenderComponent<RenderType>>()->Init(ResourceManager::Get().GetAsset<RenderType>("spritesheets\\ryu.png"));
  _borders[0]->GetComponent<StaticCollider>()->Init(Vector2<double>(0, m_nativeHeight - 40), Vector2<double>(m_nativeWidth, m_nativeHeight + 40.0f));
  _borders[0]->GetComponent<StaticCollider>()->MoveToTransform(*_borders[0]->GetComponent<Transform>());

  _borders[1] = GameManager::Get().CreateEntity<Transform, RenderComponent<RenderType>, StaticCollider>();
  _borders[1]->GetComponent<Transform>()->position.x = -100;
  _borders[1]->GetComponent<Transform>()->position.y = (float)m_nativeHeight / 2.0f;
  _borders[1]->GetComponent<RenderComponent<RenderType>>()->Init(ResourceManager::Get().GetAsset<RenderType>("spritesheets\\ryu.png"));
  _borders[1]->GetComponent<StaticCollider>()->Init(Vector2<double>(-200, 0), Vector2<double>(0, m_nativeHeight));
  _borders[1]->GetComponent<StaticCollider>()->MoveToTransform(*_borders[1]->GetComponent<Transform>());

  _borders[2] = GameManager::Get().CreateEntity<Transform, RenderComponent<RenderType>, StaticCollider>();
  _borders[2]->GetComponent<Transform>()->position.x = (float)m_nativeWidth + 100.0f;
  _borders[2]->GetComponent<Transform>()->position.y = (float)m_nativeHeight / 2.0f;
  _borders[2]->GetComponent<RenderComponent<RenderType>>()->Init(ResourceManager::Get().GetAsset<RenderType>("spritesheets\\ryu.png"));
  _borders[2]->GetComponent<StaticCollider>()->Init(Vector2<double>(m_nativeWidth, 0), Vector2<double>(m_nativeWidth + 200, m_nativeHeight));
  _borders[2]->GetComponent<StaticCollider>()->MoveToTransform(*_borders[2]->GetComponent<Transform>());

  InitCharacter(Vector2<int>(100, 0), _p1);
  InitCharacter(Vector2<int>(400, 0), _p2);

  // set up player key config
  auto kb2 = _p2->GetComponent<KeyboardInputHandler>();
  kb2->SetKey(SDLK_UP, InputState::UP);
  kb2->SetKey(SDLK_DOWN, InputState::DOWN);
  kb2->SetKey(SDLK_RIGHT, InputState::RIGHT);
  kb2->SetKey(SDLK_LEFT, InputState::LEFT);
  kb2->SetKey(SDLK_j, InputState::BTN1);
  kb2->SetKey(SDLK_k, InputState::BTN2);
  kb2->SetKey(SDLK_l, InputState::BTN3);

  _p2->RemoveComponent<KeyboardInputHandler>();
  _p2->AddComponent<GamepadInputHandler>();

  // set up camera
  _camera = GameManager::Get().CreateEntity<Camera, Transform>();
  _camera->GetComponent<Camera>()->Init(m_nativeWidth, m_nativeHeight);
}

void BattleScene::Update(float deltaTime)
{
  TimerSystem::DoTick(deltaTime);
  HitSystem::DoTick(deltaTime);

  PlayerSideSystem::DoTick(deltaTime);
  InputSystem::DoTick(deltaTime);
  GamepadInputSystem::DoTick(deltaTime);

  FrameAdvantageSystem::DoTick(deltaTime);
  // resolve collisions
  PhysicsSystem::DoTick(deltaTime);
  // update the location of the colliders
  MoveSystem::DoTick(deltaTime);

  AnimationSystem::DoTick(deltaTime);
  AttackAnimationSystem::DoTick(deltaTime);

  CheckBattleEndSystem::DoTick(deltaTime);
}


void BattleScene::InitCharacter(Vector2<float> position, std::shared_ptr<Entity> player)
{
  Vector2<int> textureSize = ResourceManager::Get().GetTextureWidthAndHeight("spritesheets\\ryu.png");
  Vector2<double> entitySize(static_cast<double>(textureSize.x) * .75, static_cast<double>(textureSize.y) * .95);

  player->AddComponents<Transform, KeyboardInputHandler, Animator, RenderComponent<RenderType>, RenderProperties, Rigidbody, GameActor, DynamicCollider, Hurtbox, StateComponent>();

  player->GetComponent<Rigidbody>()->Init(true);
  player->GetComponent<Animator>()->SetAnimations(&RyuConfig::Animations());

  player->GetComponent<Transform>()->SetWidthAndHeight(entitySize.x, entitySize.y);
  player->GetComponent<RenderProperties>()->baseRenderOffset = ((-1.0 / 2.0) * entitySize);
  player->GetComponent<RenderProperties>()->baseRenderOffset.y -= (static_cast<double>(textureSize.y) * .05);

  player->GetComponent<DynamicCollider>()->Init(Vector2<double>::Zero, entitySize);
  player->GetComponent<Hurtbox>()->Init(Vector2<double>::Zero, entitySize);

  player->SetScale(Vector2<float>(1.4f, 1.7f));
  player->GetComponent<Transform>()->position = position;

  player->GetComponent<DynamicCollider>()->MoveToTransform(*player->GetComponent<Transform>());
  player->GetComponent<Hurtbox>()->MoveToTransform(*player->GetComponent<Transform>());
}

PostMatchScene::~PostMatchScene()
{
  GameManager::Get().DestroyEntity(_borders[0]);
  GameManager::Get().DestroyEntity(_borders[1]);
  GameManager::Get().DestroyEntity(_borders[2]);
  GameManager::Get().DestroyEntity(_camera);

  // always delete transform last because it will access the other components
  _p1->RemoveComponents<Animator, RenderComponent<RenderType>, RenderProperties, Rigidbody, DynamicCollider, CutsceneActor, Transform>();
  _p2->RemoveComponents<Animator, RenderComponent<RenderType>, RenderProperties, Rigidbody, DynamicCollider, CutsceneActor, Transform>();
}

void PostMatchScene::Init(std::shared_ptr<Entity> p1, std::shared_ptr<Entity> p2)
{
  _p1 = p1;
  _p2 = p2;

  _p1->AddComponents<Animator, RenderComponent<RenderType>, RenderProperties, Rigidbody, DynamicCollider, Transform, CutsceneActor>();
  _p2->AddComponents<Animator, RenderComponent<RenderType>, RenderProperties, Rigidbody, DynamicCollider, Transform, CutsceneActor>();

  // since game actor no long exists, remove the listener
  _p1->GetComponent<Animator>()->ChangeListener(nullptr);
  _p2->GetComponent<Animator>()->ChangeListener(nullptr);

  // reset rigidbody state
  _p1->GetComponent<Rigidbody>()->elasticCollisions = false;
  _p2->GetComponent<Rigidbody>()->elasticCollisions = false;

  if(_p1->GetComponent<LoserComponent>())
  {
    _p1->GetComponent<CutsceneActor>()->SetActionList(_loserActions, 2);
    _p2->GetComponent<CutsceneActor>()->SetActionList(_winnerActions, 2);
  }
  else
  {
    _p1->GetComponent<CutsceneActor>()->SetActionList(_winnerActions, 2);
    _p2->GetComponent<CutsceneActor>()->SetActionList(_loserActions, 2);
  }

  _borders[0] = GameManager::Get().CreateEntity<Transform, RenderComponent<RenderType>, StaticCollider>();
  _borders[0]->GetComponent<Transform>()->position.x = (float)m_nativeWidth / 2.0f;
  _borders[0]->GetComponent<Transform>()->position.y = m_nativeHeight;
  _borders[0]->GetComponent<RenderComponent<RenderType>>()->Init(ResourceManager::Get().GetAsset<RenderType>("spritesheets\\ryu.png"));
  _borders[0]->GetComponent<StaticCollider>()->Init(Vector2<double>(0, m_nativeHeight - 40), Vector2<double>(m_nativeWidth, m_nativeHeight + 40.0f));
  _borders[0]->GetComponent<StaticCollider>()->MoveToTransform(*_borders[0]->GetComponent<Transform>());

  _borders[1] = GameManager::Get().CreateEntity<Transform, RenderComponent<RenderType>, StaticCollider>();
  _borders[1]->GetComponent<Transform>()->position.x = -100;
  _borders[1]->GetComponent<Transform>()->position.y = (float)m_nativeHeight / 2.0f;
  _borders[1]->GetComponent<RenderComponent<RenderType>>()->Init(ResourceManager::Get().GetAsset<RenderType>("spritesheets\\ryu.png"));
  _borders[1]->GetComponent<StaticCollider>()->Init(Vector2<double>(-200, 0), Vector2<double>(0, m_nativeHeight));
  _borders[1]->GetComponent<StaticCollider>()->MoveToTransform(*_borders[1]->GetComponent<Transform>());

  _borders[2] = GameManager::Get().CreateEntity<Transform, RenderComponent<RenderType>, StaticCollider>();
  _borders[2]->GetComponent<Transform>()->position.x = (float)m_nativeWidth + 100.0f;
  _borders[2]->GetComponent<Transform>()->position.y = (float)m_nativeHeight / 2.0f;
  _borders[2]->GetComponent<RenderComponent<RenderType>>()->Init(ResourceManager::Get().GetAsset<RenderType>("spritesheets\\ryu.png"));
  _borders[2]->GetComponent<StaticCollider>()->Init(Vector2<double>(m_nativeWidth, 0), Vector2<double>(m_nativeWidth + 200, m_nativeHeight));
  _borders[2]->GetComponent<StaticCollider>()->MoveToTransform(*_borders[2]->GetComponent<Transform>());

  // set up camera
  _camera = GameManager::Get().CreateEntity<Camera, Transform>();
  _camera->GetComponent<Camera>()->Init(m_nativeWidth, m_nativeHeight);
}

void PostMatchScene::Update(float deltaTime)
{
  CutsceneSystem::DoTick(deltaTime);
  // resolve collisions
  PhysicsSystem::DoTick(deltaTime);
  // prevent continued movement after hitting the ground
  CutsceneMovementSystem::DoTick(deltaTime);
  // update the location of the colliders
  MoveSystem::DoTick(deltaTime);

  AnimationSystem::DoTick(deltaTime);
}
