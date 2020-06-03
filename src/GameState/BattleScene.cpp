#include "GameState/Scene.h"
#include "GameManagement.h"

#include "Components/Camera.h"
#include "Components/Animator.h"
#include "Components/GameActor.h"
#include "Components/Rigidbody.h"
#include "Components/Collider.h"
#include "Components/Input.h"

#include "AssetManagement/StaticAssets/CharacterConfig.h"

void BattleScene::Init()
{
  auto bottomBorder = GameManager::Get().CreateEntity<Transform, GraphicRenderer, StaticCollider>();
  bottomBorder->GetComponent<Transform>()->position.x = (float)m_nativeWidth / 2.0f;
  bottomBorder->GetComponent<Transform>()->position.y = m_nativeHeight;
  bottomBorder->GetComponent<GraphicRenderer>()->Init(ResourceManager::Get().GetTexture("spritesheets\\ryu.png"));
  bottomBorder->GetComponent<StaticCollider>()->Init(Vector2<double>(0, m_nativeHeight - 40), Vector2<double>(m_nativeWidth, m_nativeHeight + 40.0f));
  bottomBorder->GetComponent<StaticCollider>()->MoveToTransform(*bottomBorder->GetComponent<Transform>());

  auto leftBorder = GameManager::Get().CreateEntity<Transform, GraphicRenderer, StaticCollider>();
  leftBorder->GetComponent<Transform>()->position.x = -100;
  leftBorder->GetComponent<Transform>()->position.y = (float)m_nativeHeight / 2.0f;
  leftBorder->GetComponent<GraphicRenderer>()->Init(ResourceManager::Get().GetTexture("spritesheets\\ryu.png"));
  leftBorder->GetComponent<StaticCollider>()->Init(Vector2<double>(-200, 0), Vector2<double>(0, m_nativeHeight));
  leftBorder->GetComponent<StaticCollider>()->MoveToTransform(*leftBorder->GetComponent<Transform>());

  auto rightBorder = GameManager::Get().CreateEntity<Transform, GraphicRenderer, StaticCollider>();
  rightBorder->GetComponent<Transform>()->position.x = (float)m_nativeWidth + 100.0f;
  rightBorder->GetComponent<Transform>()->position.y = (float)m_nativeHeight / 2.0f;
  rightBorder->GetComponent<GraphicRenderer>()->Init(ResourceManager::Get().GetTexture("spritesheets\\ryu.png"));
  rightBorder->GetComponent<StaticCollider>()->Init(Vector2<double>(m_nativeWidth, 0), Vector2<double>(m_nativeWidth + 200, m_nativeHeight));
  rightBorder->GetComponent<StaticCollider>()->MoveToTransform(*rightBorder->GetComponent<Transform>());

  _p1 = InitCharacter(Vector2(100, 0));
  _p2 = InitCharacter(Vector2(400, 0));

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


Camera* BattleScene::GetCamera()
{
  return _camera->GetComponent<Camera>().get();
}


std::shared_ptr<Entity> BattleScene::InitCharacter(Vector2<float> position)
{
  Vector2<int> textureSize = ResourceManager::Get().GetTextureWidthAndHeight("spritesheets\\ryu.png");
  Vector2<double> entitySize(static_cast<double>(textureSize.x) * .75, static_cast<double>(textureSize.y) * .95);

  auto player = GameManager::Get().CreateEntity<Transform, KeyboardInputHandler, Animator, GraphicRenderer, RenderProperties, Rigidbody, GameActor, DynamicCollider, Hurtbox, StateComponent>();

  player->GetComponent<Rigidbody>()->Init(true);
  player->GetComponent<Animator>()->SetAnimations(&RyuConfig::Animations());

  player->GetComponent<Transform>()->SetWidthAndHeight(entitySize.x, entitySize.y);
  player->GetComponent<RenderProperties>()->baseRenderOffset = (entitySize * (-1.0 / 2.0));
  player->GetComponent<RenderProperties>()->baseRenderOffset.y -= (static_cast<double>(textureSize.y) * .05);

  player->GetComponent<DynamicCollider>()->Init(Vector2<double>::Zero, entitySize);
  player->GetComponent<Hurtbox>()->Init(Vector2<double>::Zero, entitySize);

  player->SetScale(Vector2<float>(1.4f, 1.7f));
  player->GetComponent<Transform>()->position = position;

  player->GetComponent<DynamicCollider>()->MoveToTransform(*player->GetComponent<Transform>());
  player->GetComponent<Hurtbox>()->MoveToTransform(*player->GetComponent<Transform>());

  return player;
}