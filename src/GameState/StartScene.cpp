#include "GameState/Scene.h"
#include "GameManagement.h"
#include "ResourceManager.h"

#include "Systems/SceneSystems.h"

#include "Components/RenderComponent.h"
#include "Components/Transform.h"
#include "Components/Camera.h"
#include "Components/Input.h"
#include "Components/StateComponent.h"

IScene::~IScene()
{
  // after every scene, clear the input buffers on all devices
  auto& keyboards = ComponentManager<KeyboardInputHandler>::Get().All();
  auto& gamepads = ComponentManager<GamepadInputHandler>::Get().All();
  for(auto keyboard : keyboards)
    keyboard->ClearInputBuffer();
  for(auto gamepad : gamepads)
    gamepad->ClearInputBuffer();
}

Camera* IScene::GetCamera()
{
  return _camera->GetComponent<Camera>().get();
}

StartScene::~StartScene()
{
  GameManager::Get().DestroyEntity(_renderedText);
  GameManager::Get().DestroyEntity(_camera);
}

void StartScene::Init(std::shared_ptr<Entity> p1, std::shared_ptr<Entity> p2)
{
  _p1 = p1;
  _p2 = p2;
  
  p1->AddComponent<KeyboardInputHandler>();

  _renderedText = GameManager::Get().CreateEntity<Transform, TextRenderer, RenderProperties>();
  _renderedText->GetComponent<TextRenderer>()->SetFont(ResourceManager::Get().GetFontWriter("fonts\\Eurostile.ttf", 36));
  _renderedText->GetComponent<TextRenderer>()->SetText("PRESS ANY BUTTON TO START");

  _renderedText->GetComponent<Transform>()->position = Vector2<float>(m_nativeWidth / 2.0f, m_nativeHeight / 2.0f);

  // set up camera
  _camera = GameManager::Get().CreateEntity<Camera, Transform>();
  _camera->GetComponent<Camera>()->Init(m_nativeWidth, m_nativeHeight);
}

void StartScene::Update(float deltaTime)
{
  StartSceneInputSystem::DoTick(deltaTime);
}

CharacterSelectScene::~CharacterSelectScene()
{
  GameManager::Get().DestroyEntity(_portrait);
  GameManager::Get().DestroyEntity(_camera);
}

void CharacterSelectScene::Init(std::shared_ptr<Entity> p1, std::shared_ptr<Entity> p2)
{
  _p1 = p1;
  _p2 = p2;

  p1->AddComponent<KeyboardInputHandler>();

  //_portrait = GameManager::Get().CreateEntity<Transform, RenderComponent<RenderType>, RenderProperties>();
  _portrait = GameManager::Get().CreateEntity<Transform, TextRenderer, RenderProperties>();
  _portrait->GetComponent<TextRenderer>()->SetFont(ResourceManager::Get().GetFontWriter("fonts\\Eurostile.ttf", 36));
  _portrait->GetComponent<TextRenderer>()->SetText("Character Select");
  _portrait->GetComponent<Transform>()->position = Vector2<float>(m_nativeWidth / 2.0f, m_nativeHeight / 2.0f);

  // set up camera
  _camera = GameManager::Get().CreateEntity<Camera, Transform>();
  _camera->GetComponent<Camera>()->Init(m_nativeWidth, m_nativeHeight);
}

void CharacterSelectScene::Update(float deltaTime)
{
  CharacterSelectInputSystem::DoTick(deltaTime);
}

ResultsScene::~ResultsScene()
{
  GameManager::Get().DestroyEntity(_resultText);
  GameManager::Get().DestroyEntity(_camera);
}

void ResultsScene::Init(std::shared_ptr<Entity> p1, std::shared_ptr<Entity> p2)
{
  _p1 = p1;
  _p2 = p2;

  p1->AddComponent<KeyboardInputHandler>();

  _resultText = GameManager::Get().CreateEntity<Transform, TextRenderer, RenderProperties>();
  _resultText->GetComponent<TextRenderer>()->SetFont(ResourceManager::Get().GetFontWriter("fonts\\Eurostile.ttf", 36));
  _resultText->GetComponent<Transform>()->position = Vector2<float>(m_nativeWidth / 2.0f, m_nativeHeight / 2.0f);
  
  if(_p1->GetComponent<LoserComponent>())
  {
    _resultText->GetComponent<TextRenderer>()->SetText("PLAYER 2 WINS");
    _p1->RemoveComponent<LoserComponent>();
  }
  else if(_p2->GetComponent<LoserComponent>())
  {
    _resultText->GetComponent<TextRenderer>()->SetText("PLAYER 1 WINS");
    _p2->RemoveComponent<LoserComponent>();
  }
  else
  {
    _resultText->GetComponent<TextRenderer>()->SetText("NOBODY LOST. SOMETHING WENT HORRIBLY WRONG.");
  }
  
  // set up camera
  _camera = GameManager::Get().CreateEntity<Camera, Transform>();
  _camera->GetComponent<Camera>()->Init(m_nativeWidth, m_nativeHeight);
}

void ResultsScene::Update(float deltaTime)
{
  ResultsSceneSystem::DoTick(deltaTime);
}
