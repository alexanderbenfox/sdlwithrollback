#include "GameState/Scene.h"
#include "Managers/GameManagement.h"

#include "Components/Camera.h"
#include "Components/Animator.h"

#include "Components/Rigidbody.h"
#include "Components/Collider.h"
#include "Components/Input.h"
#include "Components/RenderComponent.h"
#include "Components/SFXComponent.h"

#include "Components/Actors/GameActor.h"
#include "Components/Actors/CutsceneActor.h"

// for wall push
#include "Components/ActionComponents.h"

#include "Systems/Physics.h"
#include "Systems/AnimationSystem.h"
#include "Systems/MoveSystem.h"
#include "Systems/InputSystem.h"
#include "Systems/HitSystem.h"
#include "Systems/TimerSystem/TimerSystem.h"
#include "Systems/CheckBattleEndSystem.h"
#include "Systems/CutsceneSystem.h"
#include "Systems/UISystem.h"
#include "Systems/AISystem.h"
#include "Systems/WallPush/WallPushSystem.h"

#include "Systems/ActionSystems/EnactActionSystem.h"
#include "Systems/ActionSystems/ActionListenerSystem.h"
#include "Systems/ActionSystems/ActionHandleInputSystem.h"
#include "Core/Prefab/ActionFactory.h"

#include "AssetManagement/AnimationCollectionManager.h"

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
  _p1->RemoveComponents<GameActor, Hurtbox, StateComponent, UIContainer, WallPushComponent, Rigidbody, Gravity, TimerContainer>();
  _p2->RemoveComponents<GameActor, Hurtbox, StateComponent, UIContainer, WallPushComponent, Rigidbody, Gravity, TimerContainer>();

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
  ActionFactory::GoToNeutralAction(_p1->GetID(), _p1->GetComponent<StateComponent>());
  ActionFactory::GoToNeutralAction(_p2->GetID(), _p2->GetComponent<StateComponent>());
  _p1->AddComponent<InputListenerComponent>();
  _p2->AddComponent<InputListenerComponent>();

  // set up camera
  _uiCamera = GameManager::Get().CreateEntity<Camera, Transform>();
  _uiCamera->GetComponent<Camera>()->Init(m_nativeWidth, m_nativeHeight);
  

  // add camera follows players component so that it is flagged for that system
  _camera = GameManager::Get().CreateEntity<Camera, Transform, CameraFollowsPlayers>();
  _camera->GetComponent<Camera>()->Init(m_nativeWidth, m_nativeHeight);
  _camera->GetComponent<Camera>()->origin = cameraOrigin;
  _camera->GetComponent<Camera>()->clamp = stage.clamp;

  GRenderer.EstablishCamera(RenderLayer::UI, _uiCamera->GetComponent<Camera>());
  GRenderer.EstablishCamera(RenderLayer::World, _camera->GetComponent<Camera>());

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

  ////++++ end state machine section ++++////

  
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
  Vector2<int> textureSize = ResourceManager::Get().GetTextureWidthAndHeight("spritesheets\\ryu.png");
  Vector2<double> entitySize(static_cast<double>(textureSize.x) * .75, static_cast<double>(textureSize.y) * .95);
  position.y = static_cast<float>(m_nativeHeight) - static_cast<float>(entitySize.y);

  // quick hack to make the debug stuff for attacks work... need to remove eventually
  player->RemoveComponents<Transform, Animator>();

  player->AddComponents<Transform, GameInputComponent, Animator, RenderComponent<RenderType>, RenderProperties, Rigidbody, Gravity, GameActor, DynamicCollider, Hurtbox, StateComponent, TeamComponent, SFXComponent>();
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
      if (lastState->hitting && newState->comboCounter > 1)
      {
        auto& activeTimers = comboTextEntity->GetComponent<TimerContainer>()->timings;
        if (!activeTimers.empty())
        {
          for (auto timer : activeTimers)
            timer->Cancel();
        }

        const int comboTextVisibleFrames = 35;
        // replace active timer with new one that will remove render properties to hide the text
        std::shared_ptr<ActionTimer> endComboText = std::shared_ptr<ActionTimer>(new SimpleActionTimer([comboTextEntity]() { comboTextEntity->RemoveComponent<RenderProperties>(); }, comboTextVisibleFrames));
        comboTextEntity->GetComponent<TimerContainer>()->timings.push_back(endComboText);

        // ensure the combo text is visible
        comboTextEntity->AddComponent<RenderProperties>();

        std::string comboText = "Combo: " + std::to_string(newState->comboCounter);
        comboTextEntity->GetComponent<TextRenderer>()->SetText(comboText);
      }
    };

    // finally, add combo text to the ui container
    uiContainer->uiComponents.push_back(_uiEntities.back()->GetComponent<UITransform>());

    if (isPlayer1)
      comboTextEntity->GetComponent<UITransform>()->parent = _p1UIAnchor->GetComponent<UITransform>();
    else
      comboTextEntity->GetComponent<UITransform>()->parent = _p2UIAnchor->GetComponent<UITransform>();
  };

  if (isPlayer1)
    createHPEntities(UIAnchor::TL, Vector2<float>::Zero);
  else
    createHPEntities(UIAnchor::TR, Vector2<float>(-200 - healthBarOffset.x * 2, 0));

  player->GetComponent<Gravity>()->force = GlobalVars::Gravity;
  player->GetComponent<Animator>()->animCollectionID = GAnimArchive.GetCollectionID("Ryu");

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


  // add this component for doing magic series
  if (!player->GetComponent<HasTargetCombo>())
  {
    player->AddComponent<HasTargetCombo>();

    auto magicSeriesMap = player->GetComponent<HasTargetCombo>();
    magicSeriesMap->links[ActionState::LIGHT] = InputState::BTN2;
    magicSeriesMap->links[ActionState::MEDIUM] = InputState::BTN3;
  }

  //! Janky loading
  player->GetComponent<SFXComponent>()->ShowHitSparks(false);
  player->GetComponent<SFXComponent>()->ShowBlockSparks(false);

}

PostMatchScene::~PostMatchScene()
{
  GameManager::Get().DestroyEntity(_borders[0]);
  GameManager::Get().DestroyEntity(_borders[1]);
  GameManager::Get().DestroyEntity(_borders[2]);
  GameManager::Get().DestroyEntity(_uiCamera);
  GameManager::Get().DestroyEntity(_camera);

  // always delete transform last because it will access the other components
  // in this case, no longer 'acting' so actor comp can be removed
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
  GRenderer.EstablishCamera(RenderLayer::UI, _uiCamera->GetComponent<Camera>());

  // add camera follows players component so that it is flagged for that system
  _camera = GameManager::Get().CreateEntity<Camera, Transform, CameraFollowsPlayers>();
  _camera->GetComponent<Camera>()->Init(m_nativeWidth, m_nativeHeight);
  _camera->GetComponent<Camera>()->origin = cameraOrigin;
  _camera->GetComponent<Camera>()->clamp = stage.clamp;
  GRenderer.EstablishCamera(RenderLayer::World, _camera->GetComponent<Camera>());
}

void PostMatchScene::Update(float deltaTime)
{
  UIPositionUpdateSystem::DoTick(deltaTime);

  CutsceneSystem::DoTick(deltaTime);
  // resolve collisions
  ApplyGravitySystem::DoTick(deltaTime);
  PhysicsSystem::DoTick(deltaTime);
  // prevent continued movement after hitting the ground
  CutsceneMovementSystem::DoTick(deltaTime);
  // update the location of the colliders
  MoveSystem::DoTick(deltaTime);

  AnimationSystem::DoTick(deltaTime);
}
