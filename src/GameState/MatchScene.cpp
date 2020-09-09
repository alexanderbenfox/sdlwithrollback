#include "GameState/MatchScene.h"
#include "GameState/BattleScene.h"

#include "Managers/GameManagement.h"

#include "Components/Rigidbody.h"
#include "Components/Collider.h"
#include "Components/Actors/CutsceneActor.h"
#include "Components/MetaGameComponents.h"
#include "Components/Camera.h"
#include "Systems/TimerSystem/TimerContainer.h"

#include "Systems/CutsceneSystem.h"
#include "Systems/AnimationSystem.h"
#include "Systems/Physics.h"
#include "Systems/UISystem.h"
#include "Systems/MoveSystem.h"
#include "Systems/DestroyEntitiesSystem.h"
#include "Systems/TimerSystem/TimerSystem.h"
#include "Systems/InputSystem.h"
#include "Systems/ActionSystems/EnactActionSystem.h"

#include "Core/Prefab/CharacterConstructor.h"
#include "Core/Prefab/ActionFactory.h"

PreMatchScene::PreMatchScene(MatchMetaComponent& matchData) :
  _fadeAction1(1.5f, 255, 0),
  _fadeAction2(0.5f, 0, 255),
  _wait1(3),
  _wait2(4),
  _waitForEntranceComplete(2),
  _pauseP1(1.2f),
  _pauseP2(1.2f),
  _entranceAction("Entrance"),
  _idle("Idle"),
  ISubScene(matchData)
{}

PreMatchScene::~PreMatchScene()
{
  _p1->RemoveComponents<CutsceneActor, TimerContainer>();
  _p2->RemoveComponents<CutsceneActor, TimerContainer>();
}

void PreMatchScene::Init(std::shared_ptr<Entity> p1, std::shared_ptr<Entity> p2)
{
  _p1 = p1;
  _p2 = p2;

  CharacterConstructor::InitSpatialComponents(_p1, Vector2<int>(100, 0));
  CharacterConstructor::InitSpatialComponents(_p2, Vector2<int>(400, 0));

  _p1->GetComponent<TeamComponent>()->team = TeamComponent::Team::TeamA;
  _p2->GetComponent<TeamComponent>()->team = TeamComponent::Team::TeamB;

  _p1->GetComponent<StateComponent>()->onLeftSide = false;
  _p2->GetComponent<StateComponent>()->onLeftSide = true;

  _p1->GetComponent<RenderProperties>()->horizontalFlip= false;
  _p2->GetComponent<RenderProperties>()->horizontalFlip = true;

  _p1->AddComponents<CutsceneActor, TimerContainer>();
  _p2->AddComponents<CutsceneActor, TimerContainer>();

  _pauseP1.target = _p1;
  _pauseP2.target = _p2;

  _p1->GetComponent<CutsceneActor>()->SetActionList(_PCEntranceActionSet1, 3);
  _p2->GetComponent<CutsceneActor>()->SetActionList(_PCEntranceActionSet2, 3);


  _roundText = GameManager::Get().CreateEntity<UITransform, TextRenderer, TimerContainer, DestroyOnSceneEnd>();
  _roundText->GetComponent<TextRenderer>()->SetFont(ResourceManager::Get().GetFontWriter("fonts\\RUBBBB__.TTF", 36));
  std::string roundString = "ROUND " + std::to_string(_matchStatus.roundNo + 1);
  _roundText->GetComponent<TextRenderer>()->SetText(roundString, TextAlignment::Centered);
  _roundText->GetComponent<UITransform>()->anchor = UIAnchor::Center;

  _fightText = GameManager::Get().CreateEntity<UITransform, TextRenderer, TimerContainer, DestroyOnSceneEnd>();
  _fightText->GetComponent<TextRenderer>()->SetFont(ResourceManager::Get().GetFontWriter("fonts\\RUBBBB__.TTF", 52));
  _fightText->GetComponent<TextRenderer>()->SetText("FIGHT", TextAlignment::Centered);
  _fightText->GetComponent<UITransform>()->anchor = UIAnchor::Center;

  _fadeAction1.target = _roundText;
  _fadeAction2.target = _fightText;

  // add these components so they will be ran through the cutscene system (check on this)
  _roundText->AddComponents<RenderProperties, CutsceneActor, RenderComponent<RenderType>, Animator>();
  _fightText->AddComponents<RenderProperties, CutsceneActor, RenderComponent<RenderType>, Animator>();
  _roundText->GetComponent<CutsceneActor>()->SetActionList(_fadeActionSet1, 3);
  _fightText->GetComponent<CutsceneActor>()->SetActionList(_fadeActionSet2, 2);

  // hide the fight text
  _fightText->GetComponent<RenderProperties>()->SetDisplayColor(255, 255, 255, 0);

  // play idle before going into cutscene actions
  _p1->GetComponent<Animator>()->Play("Idle", true, 1.0f, true);

  // 
  Animation* anim = _p2->GetComponent<Animator>()->Play("Idle", true, 1.0f, true);
  RenderProperties& properties = *_p2->GetComponent<RenderProperties>();
  Animator& animator = *_p2->GetComponent<Animator>();
  properties.offset = -GAnimArchive.GetCollection(animator.animCollectionID).GetRenderOffset("Idle", true, (int)std::floor(properties.unscaledRenderWidth));
}

void PreMatchScene::Update(float deltaTime)
{
  PlayerSideSystem::DoTick(deltaTime);
  UIPositionUpdateSystem::DoTick(deltaTime);

  CutsceneSystem::DoTick(deltaTime);
  // resolve collisions
  ApplyGravitySystem::DoTick(deltaTime);
  PhysicsSystem::DoTick(deltaTime);
  // prevent continued movement after hitting the ground
  //CutsceneMovementSystem::DoTick(deltaTime);
  // update the location of the colliders
  MoveSystem::DoTick(deltaTime);

  AnimationSystem::DoTick(deltaTime);

  // update AI timers, UI timers (all non-state timers)
  TimerSystem::DoTick(deltaTime);
}

PostMatchScene::PostMatchScene(MatchMetaComponent& matchData) :
  winnerAction1(3),
  winnerAction2("Win"),
  loserAction1("KO"),
  loserAction2(5),
  KODisplay(0.6f),
  poseHold(0.5f),
  fadeOutKO(0.05f, 255, 0),
  waitForText(2),
  ISubScene(matchData)
{}

PostMatchScene::~PostMatchScene()
{
  // always delete transform last because it will access the other components
  // in this case, no longer 'acting' so actor comp can be removed
  _p1->RemoveComponents<Animator, RenderComponent<RenderType>, RenderProperties, Rigidbody, DynamicCollider, CutsceneActor, Transform, TimerContainer>();
  _p2->RemoveComponents<Animator, RenderComponent<RenderType>, RenderProperties, Rigidbody, DynamicCollider, CutsceneActor, Transform, TimerContainer>();
}

void PostMatchScene::Init(std::shared_ptr<Entity> p1, std::shared_ptr<Entity> p2)
{
  _p1 = p1;
  _p2 = p2;

  _p1->AddComponents<Animator, RenderComponent<RenderType>, RenderProperties, Rigidbody, DynamicCollider, Transform, CutsceneActor, TimerContainer>();
  _p2->AddComponents<Animator, RenderComponent<RenderType>, RenderProperties, Rigidbody, DynamicCollider, Transform, CutsceneActor, TimerContainer>();

  // since game actor no long exists, remove the listener
  _p1->GetComponent<Animator>()->ChangeListener(nullptr);
  _p2->GetComponent<Animator>()->ChangeListener(nullptr);

  // reset rigidbody state
  _p1->GetComponent<Rigidbody>()->elasticCollisions = false;
  _p2->GetComponent<Rigidbody>()->elasticCollisions = false;

  if (_p1->GetComponent<LoserComponent>())
  {
    poseHold.target = _p2;
    _p1->GetComponent<CutsceneActor>()->SetActionList(_loserActions, 2);
    _p2->GetComponent<CutsceneActor>()->SetActionList(_winnerActions, 4);
  }
  else
  {
    poseHold.target = _p1;
    _p1->GetComponent<CutsceneActor>()->SetActionList(_winnerActions, 4);
    _p2->GetComponent<CutsceneActor>()->SetActionList(_loserActions, 2);
  }


  _koText = GameManager::Get().CreateEntity<UITransform, TextRenderer, TimerContainer, DestroyOnSceneEnd>();
  _koText->GetComponent<TextRenderer>()->SetFont(ResourceManager::Get().GetFontWriter("fonts\\Eurostile.ttf", 36));
  _koText->GetComponent<TextRenderer>()->SetText("KO!", TextAlignment::Centered);
  _koText->GetComponent<UITransform>()->anchor = UIAnchor::Center;

  KODisplay.target = _koText;
  fadeOutKO.target = _koText;

  // add these components so they will be ran through the cutscene system (check on this)
  _koText->AddComponents<RenderProperties, CutsceneActor, RenderComponent<RenderType>, Animator>();
  _koText->GetComponent<CutsceneActor>()->SetActionList(_textActions, 2);
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

  // update AI timers, UI timers (all non-state timers)
  TimerSystem::DoTick(deltaTime);
}

MatchScene::~MatchScene()
{
  _subScene.reset();

  GameManager::Get().DestroyEntity(_uiCamera);
  GameManager::Get().DestroyEntity(_camera);
  GameManager::Get().DestroyEntity(_stageBorders[0]);
  GameManager::Get().DestroyEntity(_stageBorders[1]);
  GameManager::Get().DestroyEntity(_stageBorders[2]);
  GameManager::Get().DestroyEntity(_matchData);

  //GRenderer.EstablishCamera(RenderLayer::UI, nullptr);
  //GRenderer.EstablishCamera(RenderLayer::World, nullptr);
}

void MatchScene::Init(std::shared_ptr<Entity> p1, std::shared_ptr<Entity> p2)
{
  _p1 = p1;
  _p2 = p2;

  // set up battle stage
  Rect<float> stageRect = Rect<float>(-m_nativeWidth, 0, 2 * m_nativeWidth, m_nativeHeight);
  BattleScene::StageBorders stage = BattleScene::CreateStageBorders(stageRect, m_nativeWidth, m_nativeHeight);
  _stageBorders[0] = stage.borders[0];
  _stageBorders[1] = stage.borders[1];
  _stageBorders[2] = stage.borders[2];

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

  // create match metadata
  _matchData = GameManager::Get().CreateEntity<MatchMetaComponent>();
  MatchMetaComponent& matchStatus = *_matchData->GetComponent<MatchMetaComponent>();

  if (_battleType == BattleType::Training)
  {
    _subScene = std::make_shared<BattleScene>(matchStatus);
    _subScene->Init(_p1, _p2);

    _p1->GetComponent<StateComponent>()->invulnerable = true;
    _p2->GetComponent<StateComponent>()->invulnerable = true;
    _currStage = MatchStage::BATTLE;
  }
  else
  {
    _subScene = std::make_shared<PreMatchScene>(matchStatus);
    _subScene->Init(_p1, _p2);

    _p1->GetComponent<StateComponent>()->invulnerable = false;
    _p2->GetComponent<StateComponent>()->invulnerable = false;
    _currStage = MatchStage::PREMATCH;
  }
}

void MatchScene::Update(float deltaTime)
{
  _subScene->Update(deltaTime);
}

void MatchScene::AdvanceScene()
{
  //! read the current match status
  MatchMetaComponent& matchStatus = *_matchData->GetComponent<MatchMetaComponent>();

  auto nextBattleScene = [this, &matchStatus](int nRounds)
  {


    if (_currStage == MatchStage::POSTMATCH)
    {
      if (matchStatus.scoreP1 >= nRounds || matchStatus.scoreP2 >= nRounds)
      {
        _p1->RemoveComponent<LoserComponent>();
        _p2->RemoveComponent<LoserComponent>();
        if (matchStatus.scoreP1 > matchStatus.scoreP2)
          _p2->AddComponent<LoserComponent>();
        else
          _p1->AddComponent<LoserComponent>();

        _subScene.reset();
        GameManager::Get().RequestSceneChange(SceneType::RESULTS);
      }
      else
      {
        _p1->RemoveComponent<LoserComponent>();
        _p2->RemoveComponent<LoserComponent>();

        // advance round no before instantiating next scene
        matchStatus.roundNo++;

        _subScene = std::make_shared<PreMatchScene>(matchStatus);
        _subScene->Init(_p1, _p2);
        _currStage = MatchStage::PREMATCH;
      }
    }
    else if (_currStage == MatchStage::PREMATCH)
    {
      _subScene = std::make_shared<BattleScene>(matchStatus);
      _subScene->Init(_p1, _p2);

      _currStage = MatchStage::BATTLE;
    }
    else
    {
      _subScene = std::make_shared<PostMatchScene>(matchStatus);
      _subScene->Init(_p1, _p2);
      if (_p1->GetComponent<LoserComponent>())
        matchStatus.scoreP2++;
      else
        matchStatus.scoreP1++;
      _currStage = MatchStage::POSTMATCH;
    }
  };

  if (_battleType == BattleType::Training)
  {
    _subScene = std::make_shared<BattleScene>(matchStatus);
    _subScene->Init(_p1, _p2);
    _p1->GetComponent<StateComponent>()->invulnerable = true;
    _p2->GetComponent<StateComponent>()->invulnerable = true;

  }
  else if (_battleType == BattleType::BestOf3)
  {
    nextBattleScene(2);
  }
  else if (_battleType == BattleType::BestOf5)
  {
    nextBattleScene(3);
  }
}
