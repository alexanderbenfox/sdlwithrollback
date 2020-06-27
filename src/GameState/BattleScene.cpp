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
#include "Systems/UISystem.h"

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
  for (int i = 0; i < _uiEntities.size(); i++)
    GameManager::Get().DestroyEntity(_uiEntities[i]);
  GameManager::Get().DestroyEntity(_camera);

  // we are moving into the after match cutscene, so only remove game state related components
  _p1->RemoveComponents<GameActor, Hurtbox, StateComponent, UIContainer, TimerContainer>();
  _p2->RemoveComponents<GameActor, Hurtbox, StateComponent, UIContainer, TimerContainer>();
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

  InitCharacter(Vector2<int>(100, 0), _p1, true);
  InitCharacter(Vector2<int>(400, 0), _p2, false);

  // set the combo text anchors to the other side
  _p2ComboText->GetComponent<UITransform>()->parent = _p1UIAnchor->GetComponent<UITransform>();
  _p1ComboText->GetComponent<UITransform>()->parent = _p2UIAnchor->GetComponent<UITransform>();

  // set up camera
  _camera = GameManager::Get().CreateEntity<Camera, Transform>();
  _camera->GetComponent<Camera>()->Init(m_nativeWidth, m_nativeHeight);
}

void BattleScene::Update(float deltaTime)
{
  UIPositionUpdateSystem::DoTick(deltaTime);
  UIContainerUpdateSystem::DoTick(deltaTime);

  TimerSystem::DoTick(deltaTime);
  HitSystem::DoTick(deltaTime);

  PlayerSideSystem::DoTick(deltaTime);
  InputSystem::DoTick(deltaTime);

  FrameAdvantageSystem::DoTick(deltaTime);
  // resolve collisions
  PhysicsSystem::DoTick(deltaTime);
  // update the location of the colliders
  MoveSystem::DoTick(deltaTime);

  AnimationSystem::DoTick(deltaTime);
  AttackAnimationSystem::DoTick(deltaTime);

  CheckBattleEndSystem::DoTick(deltaTime);
}


void BattleScene::InitCharacter(Vector2<float> position, std::shared_ptr<Entity> player, bool isPlayer1)
{
  Vector2<int> textureSize = ResourceManager::Get().GetTextureWidthAndHeight("spritesheets\\ryu.png");
  Vector2<double> entitySize(static_cast<double>(textureSize.x) * .75, static_cast<double>(textureSize.y) * .95);

  player->AddComponents<Transform, GameInputComponent, Animator, RenderComponent<RenderType>, RenderProperties, Rigidbody, GameActor, DynamicCollider, Hurtbox, StateComponent>();
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
      dynamic_cast<UIRectangleRenderComponent*>(comp)->shownSize.w = static_cast<int>((float)lifeBarSize.x * p);
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
    int& comboCounter = isPlayer1 ? _comboCounterP1 : _comboCounterP2;

    // set the ui data transfer callback
    _uiEntities.back()->GetComponent<UITransform>()->callback = [comboTextEntity, &comboCounter](const StateComponent* lastState, const StateComponent* newState, UIComponent* comp)
    {
      if (newState->onNewState)
      {
        if (newState->actionState != ActionState::HITSTUN && lastState->actionState == ActionState::HITSTUN)
        {
          comboCounter = 0;
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

        if (newState->actionState == ActionState::HITSTUN)
          comboCounter++;

        std::string comboText = "Combo: " + std::to_string(comboCounter);
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
