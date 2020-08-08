#include "GameState/Scene.h"
#include "GameManagement.h"

#include "Components/Camera.h"
#include "Components/Animator.h"
#include "Components/GameActor.h"
#include "Components/Rigidbody.h"
#include "Components/Collider.h"
#include "Components/Input.h"
#include "Components/RenderComponent.h"
#include "Components/SFXComponent.h"

#include "Systems/Physics.h"
#include "Systems/AnimationSystem.h"
#include "Systems/MoveSystem.h"
#include "Systems/InputSystem.h"
#include "Systems/HitSystem.h"
#include "Systems/TimerSystem.h"
#include "Systems/CheckBattleEndSystem.h"
#include "Systems/CutsceneSystem.h"
#include "Systems/UISystem.h"
#include "Systems/AISystem.h"

#include "Systems/ActionSystems/EnactActionSystem.h"
#include "Systems/ActionSystems/ActionListenerSystem.h"
#include "Systems/ActionSystems/ActionHandleInputSystem.h"
#include "Core/Prefab/ActionFactory.h"

#include "AssetManagement/StaticAssets/CharacterConfig.h"

const Vector2<float> cameraOrigin(m_nativeWidth / 2.0f, m_nativeHeight / 2.0f);

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
  for (int i = 0; i < _uiEntities.size(); i++)
    GameManager::Get().DestroyEntity(_uiEntities[i]);
  GameManager::Get().DestroyEntity(_uiCamera);
  GameManager::Get().DestroyEntity(_camera);

  // we are moving into the after match cutscene, so only remove game state related components
  _p1->RemoveComponents<GameActor, Hurtbox, StateComponent, UIContainer, PushComponent, Rigidbody, TimerContainer>();
  _p2->RemoveComponents<GameActor, Hurtbox, StateComponent, UIContainer, PushComponent, Rigidbody, TimerContainer>();

  //_p1->RemoveComponents<Animator, RenderComponent<RenderType>, RenderProperties, Rigidbody, GameActor, DynamicCollider, Hurtbox, StateComponent, UIContainer, TimerContainer, Transform>();
  //_p1->RemoveComponents<Animator, RenderComponent<RenderType>, RenderProperties, Rigidbody, GameActor, DynamicCollider, Hurtbox, StateComponent, UIContainer, TimerContainer, Transform>();

}

void BattleScene::Init(std::shared_ptr<Entity> p1, std::shared_ptr<Entity> p2)
{
  _p1 = p1;
  _p2 = p2;

  Rect<float> stageRect = Rect<float>(-m_nativeWidth, 0, 2 * m_nativeWidth, m_nativeHeight);
  StageBorders stage = CreateStageBorders(stageRect, m_nativeWidth, m_nativeHeight);
  _borders[0] = stage.borders[0];
  _borders[1] = stage.borders[1];
  _borders[2] = stage.borders[2];

  InitCharacter(Vector2<int>(100, 0), _p1, true);
  InitCharacter(Vector2<int>(400, 0), _p2, false);

  //set player state to neutral
  ActionFactory::GoToNeutralAction(_p1.get(), _p1->GetComponent<StateComponent>().get());
  ActionFactory::GoToNeutralAction(_p2.get(), _p2->GetComponent<StateComponent>().get());
  ActionFactory::DisableActionListenerForEntities();
  _p1->AddComponent<InputListenerComponent>();
  _p2->AddComponent<InputListenerComponent>();

  // set the combo text anchors to the other side
  _p2ComboText->GetComponent<UITransform>()->parent = _p1UIAnchor->GetComponent<UITransform>();
  _p1ComboText->GetComponent<UITransform>()->parent = _p2UIAnchor->GetComponent<UITransform>();

  // set up camera
  _uiCamera = GameManager::Get().CreateEntity<Camera, Transform>();
  _uiCamera->GetComponent<Camera>()->Init(m_nativeWidth, m_nativeHeight);
  GRenderer.EstablishCamera(RenderLayer::UI, _uiCamera->GetComponent<Camera>().get());

  _camera = GameManager::Get().CreateEntity<Camera, Transform>();
  _camera->GetComponent<Camera>()->Init(m_nativeWidth, m_nativeHeight);
  _camera->GetComponent<Camera>()->followPlayers = true;
  _camera->GetComponent<Camera>()->player1 = _p1;
  _camera->GetComponent<Camera>()->player2 = _p2;
  _camera->GetComponent<Camera>()->origin = cameraOrigin;
  _camera->GetComponent<Camera>()->clamp = stage.clamp;
  GRenderer.EstablishCamera(RenderLayer::World, _camera->GetComponent<Camera>().get());

}

void BattleScene::Update(float deltaTime)
{
  UIPositionUpdateSystem::DoTick(deltaTime);
  UIContainerUpdateSystem::DoTick(deltaTime);

  // update timer systems together
  TimedActionSystem::DoTick(deltaTime);
  TimerSystem::DoTick(deltaTime);

  HitSystem::DoTick(deltaTime);
  ThrowSystem::DoTick(deltaTime);
  PushSystem::DoTick(deltaTime);

  
  UpdateAISystem::DoTick(deltaTime);

  //

  InputSystem::DoTick(deltaTime);
  // update player side system here cause it forces new state
  PlayerSideSystem::DoTick(deltaTime);

  HandleUpdateAggregate::DoTick(deltaTime);

  //

  FrameAdvantageSystem::DoTick(deltaTime);

  AnimationListenerSystem::DoTick(deltaTime);
  AttackAnimationSystem::DoTick(deltaTime);
  AnimationSystem::DoTick(deltaTime);
  
  // resolve collisions
  PhysicsSystem::DoTick(deltaTime);
  // update the location of the colliders
  MoveSystem::DoTick(deltaTime);
  // move walls according to camera position
  MoveWallSystem::DoTick(deltaTime);

  // Check action transitions after physics
  StateTransitionAggregate::DoTick(deltaTime);

  // Enact new action states then clean them up
  EnactAggregate::DoTick(deltaTime);
  CleanUpActionSystem::PostUpdate();

  CheckBattleEndSystem::DoTick(deltaTime);
}

BattleScene::StageBorders BattleScene::CreateStageBorders(const Rect<float>& stageRect, int screenWidth, int screenHeight)
{
  StageBorders stage;
  stage.clamp = Rect<float>(stageRect.beg.x + screenWidth / 2, 0, stageRect.end.x - screenWidth / 2, stageRect.end.y - screenHeight / 2);

  const float borderWidth = 200;
  const float borderHeight = 80;

  stage.borders[0] = GameManager::Get().CreateEntity<Transform, RenderComponent<RenderType>, StaticCollider>();
  stage.borders[0]->GetComponent<Transform>()->position.x = (stageRect.beg.x + stageRect.end.x) / 2.0f;
  stage.borders[0]->GetComponent<Transform>()->position.y = stageRect.end.y;
  stage.borders[0]->GetComponent<RenderComponent<RenderType>>()->Init(ResourceManager::Get().GetAsset<RenderType>("spritesheets\\ryu.png"));
  stage.borders[0]->GetComponent<StaticCollider>()->Init(Vector2<double>(stageRect.beg.x, stageRect.end.y - borderHeight / 2.0f), Vector2<double>(stageRect.end.x, stageRect.end.y + borderHeight / 2.0f));
  stage.borders[0]->GetComponent<StaticCollider>()->MoveToTransform(*stage.borders[0]->GetComponent<Transform>());

  stage.borders[1] = GameManager::Get().CreateEntity<Transform, RenderComponent<RenderType>, StaticCollider>();
  stage.borders[1]->GetComponent<Transform>()->position.x = stageRect.beg.x - 100;
  stage.borders[1]->GetComponent<Transform>()->position.y = (stageRect.beg.y + stageRect.end.y) / 2.0f;
  stage.borders[1]->GetComponent<RenderComponent<RenderType>>()->Init(ResourceManager::Get().GetAsset<RenderType>("spritesheets\\ryu.png"));
  stage.borders[1]->GetComponent<StaticCollider>()->Init(Vector2<double>(-borderWidth, 0), Vector2<double>(0, stageRect.Height()));
  stage.borders[1]->GetComponent<StaticCollider>()->MoveToTransform(*stage.borders[1]->GetComponent<Transform>());
  stage.borders[1]->AddComponent<WallMoveComponent>();
  stage.borders[1]->GetComponent<WallMoveComponent>()->leftWall = true;

  stage.borders[2] = GameManager::Get().CreateEntity<Transform, RenderComponent<RenderType>, StaticCollider>();
  stage.borders[2]->GetComponent<Transform>()->position.x = stageRect.end.x + 100.0f;
  stage.borders[2]->GetComponent<Transform>()->position.y = (stageRect.beg.y + stageRect.end.y) / 2.0f;
  stage.borders[2]->GetComponent<RenderComponent<RenderType>>()->Init(ResourceManager::Get().GetAsset<RenderType>("spritesheets\\ryu.png"));
  stage.borders[2]->GetComponent<StaticCollider>()->Init(Vector2<double>(0, 0), Vector2<double>(borderWidth, stageRect.Height()));
  stage.borders[2]->GetComponent<StaticCollider>()->MoveToTransform(*stage.borders[2]->GetComponent<Transform>());
  stage.borders[2]->AddComponent<WallMoveComponent>();
  stage.borders[2]->GetComponent<WallMoveComponent>()->leftWall = false;
  
  return stage;
}


void BattleScene::InitCharacter(Vector2<float> position, std::shared_ptr<Entity> player, bool isPlayer1)
{
  Vector2<int> textureSize = ResourceManager::Get().GetTextureWidthAndHeight("spritesheets\\ryu.png");
  Vector2<double> entitySize(static_cast<double>(textureSize.x) * .75, static_cast<double>(textureSize.y) * .95);
  position.y = static_cast<float>(m_nativeHeight) - static_cast<float>(entitySize.y);

  // quick hack to make the debug stuff for attacks work... need to remove eventually
  player->RemoveComponents<Transform, Animator>();

  player->AddComponents<Transform, GameInputComponent, Animator, RenderComponent<RenderType>, RenderProperties, Rigidbody, GameActor, DynamicCollider, Hurtbox, StateComponent, TeamComponent, SFXComponent>();
  player->AddComponents<UIContainer, TimerContainer>();

  auto uiContainer = player->GetComponent<UIContainer>();
  const Vector2<float> healthBarOffset = { 30.0f, 20.0f };

  const Vector2<int> lifeBarSize = { 200, 25 };
  const Vector2<int> margin = { 2, 2 };

  auto createHPEntities = [this, lifeBarSize, margin, healthBarOffset, &uiContainer, isPlayer1](UIAnchor anchor, Vector2<float> offset)
  {
    // set up outline first
    _uiEntities.push_back(GameManager::Get().CreateEntity<UITransform, RenderProperties>());
    _uiEntities.back()->GetComponent<UITransform>()->rect = Rect<float>(0, 0, lifeBarSize.x + 2 * margin.x, lifeBarSize.y + 2 * margin.y);
    _uiEntities.back()->GetComponent<UITransform>()->position = { healthBarOffset.x + offset.x, healthBarOffset.y + offset.y};
    _uiEntities.back()->GetComponent<UITransform>()->anchor = anchor;

    _uiEntities.back()->AddComponent<UIRectangleRenderComponent>();

    std::shared_ptr<Entity> outline = _uiEntities.back();
    if (isPlayer1)
      _p1UIAnchor = outline;
    else
      _p2UIAnchor = outline;

    // now create a new entity for the fill
    _uiEntities.push_back(GameManager::Get().CreateEntity<UITransform, RenderProperties>());
    _uiEntities.back()->GetComponent<UITransform>()->rect = Rect<float>(0, 0, lifeBarSize.x, lifeBarSize.y);
    _uiEntities.back()->GetComponent<UITransform>()->position = margin;

    _uiEntities.back()->GetComponent<UITransform>()->anchor = UIAnchor::TL;
    _uiEntities.back()->GetComponent<UITransform>()->parent = outline->GetComponent<UITransform>();

    _uiEntities.back()->GetComponent<RenderProperties>()->SetDisplayColor(255, 0, 0);

    _uiEntities.back()->AddComponent<UIRectangleRenderComponent>();
    _uiEntities.back()->GetComponent<UIRectangleRenderComponent>()->isFilled = true;
    _uiEntities.back()->GetComponent<UIRectangleRenderComponent>()->callback = [lifeBarSize](const StateComponent* lastState, const StateComponent* info, UIComponent* comp)
    {
      float p = (float)info->hp / 100.0f;
      dynamic_cast<UIRectangleRenderComponent*>(comp)->shownSize.w = (float)lifeBarSize.x * p;
    };

    uiContainer->uiComponents.push_back(_uiEntities.back()->GetComponent<UIRectangleRenderComponent>());

    // add the combo hit counter entity
    // create the counter text without render properties so that it wont be visible
    _uiEntities.push_back(GameManager::Get().CreateEntity<UITransform, TextRenderer, TimerContainer>());
    _uiEntities.back()->GetComponent<TextRenderer>()->SetFont(ResourceManager::Get().GetFontWriter("fonts\\Eurostile.ttf", 36));

    // set parent transform and offset
    _uiEntities.back()->GetComponent<UITransform>()->parent = outline->GetComponent<UITransform>();
    _uiEntities.back()->GetComponent<UITransform>()->anchor = UIAnchor::BL;
    _uiEntities.back()->GetComponent<UITransform>()->position = Vector2<float>(5.0f, 20.0f);
    std::shared_ptr<Entity> comboTextEntity = _uiEntities.back();

    // set the ui data transfer callback
    _uiEntities.back()->GetComponent<UITransform>()->callback = [comboTextEntity](const StateComponent* lastState, const StateComponent* newState, UIComponent* comp)
    {
      if (newState->onNewState)
      {
        if (newState->actionState != ActionState::HITSTUN && lastState->actionState == ActionState::HITSTUN)
        {
          // remove render properties to hide the text
          std::shared_ptr<ActionTimer> endComboText = std::shared_ptr<ActionTimer>(new SimpleActionTimer(
            [comboTextEntity]() { comboTextEntity->RemoveComponent<RenderProperties>(); },
            5));
          comboTextEntity->GetComponent<TimerContainer>()->timings.push_back(endComboText);
        }
        else if (newState->actionState == ActionState::HITSTUN)
        {
          comboTextEntity->AddComponent<RenderProperties>();
        }

        std::string comboText = "Combo: " + std::to_string(newState->comboCounter);
        comboTextEntity->GetComponent<TextRenderer>()->SetText(comboText);
      }
    };

    // finally, add combo text to the ui container
    uiContainer->uiComponents.push_back(_uiEntities.back()->GetComponent<UITransform>());

    if (isPlayer1)
      _p1ComboText = comboTextEntity;
    else
      _p2ComboText = comboTextEntity;
  };

  if (isPlayer1)
    createHPEntities(UIAnchor::TL, Vector2<float>::Zero);
  else
    createHPEntities(UIAnchor::TR, Vector2<float>(-200 - healthBarOffset.x * 2, 0));

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

  if (isPlayer1)
    player->GetComponent<TeamComponent>()->team = TeamComponent::Team::TeamA;
  else
    player->GetComponent<TeamComponent>()->team = TeamComponent::Team::TeamB;

  player->GetComponent<TeamComponent>()->playerEntity = true;


  //! Janky loading
  player->GetComponent<SFXComponent>()->ShowHitSparks();
  player->GetComponent<SFXComponent>()->ShowBlockSparks();
}

PostMatchScene::~PostMatchScene()
{
  GameManager::Get().DestroyEntity(_borders[0]);
  GameManager::Get().DestroyEntity(_borders[1]);
  GameManager::Get().DestroyEntity(_borders[2]);
  GameManager::Get().DestroyEntity(_uiCamera);
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

  Rect<float> stageRect = Rect<float>(-m_nativeWidth, 0, 2 * m_nativeWidth, m_nativeHeight);
  BattleScene::StageBorders stage = BattleScene::CreateStageBorders(stageRect, m_nativeWidth, m_nativeHeight);
  _borders[0] = stage.borders[0];
  _borders[1] = stage.borders[1];
  _borders[2] = stage.borders[2];

  // set up camera
  _uiCamera = GameManager::Get().CreateEntity<Camera, Transform>();
  _uiCamera->GetComponent<Camera>()->Init(m_nativeWidth, m_nativeHeight);
  GRenderer.EstablishCamera(RenderLayer::UI, _uiCamera->GetComponent<Camera>().get());

  _camera = GameManager::Get().CreateEntity<Camera, Transform>();
  _camera->GetComponent<Camera>()->Init(m_nativeWidth, m_nativeHeight);
  _camera->GetComponent<Camera>()->followPlayers = true;
  _camera->GetComponent<Camera>()->player1 = _p1;
  _camera->GetComponent<Camera>()->player2 = _p2;
  _camera->GetComponent<Camera>()->origin = cameraOrigin;
  _camera->GetComponent<Camera>()->clamp = stage.clamp;
  GRenderer.EstablishCamera(RenderLayer::World, _camera->GetComponent<Camera>().get());
}

void PostMatchScene::Update(float deltaTime)
{
  UIPositionUpdateSystem::DoTick(deltaTime);

  CutsceneSystem::DoTick(deltaTime);
  // resolve collisions
  PhysicsSystem::DoTick(deltaTime);
  // prevent continued movement after hitting the ground
  CutsceneMovementSystem::DoTick(deltaTime);
  // update the location of the colliders
  MoveSystem::DoTick(deltaTime);

  AnimationSystem::DoTick(deltaTime);
}
