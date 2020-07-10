#include "GameState/Scene.h"
#include "GameManagement.h"
#include "ResourceManager.h"

#include "Systems/SceneSystems.h"
#include "Systems/UISystem.h"
#include "Systems/MoveSystem.h"

#include "Components/RenderComponent.h"
#include "Components/Transform.h"
#include "Components/Camera.h"
#include "Components/Input.h"
#include "Components/StateComponent.h"

IScene::~IScene()
{
  // after every scene, clear the input buffers on all devices
  auto& inputSources = ComponentManager<GameInputComponent>::Get().All();
  for(auto source : inputSources)
    source->Clear();
}

StartScene::~StartScene()
{
  GameManager::Get().DestroyEntity(_renderedText);
  GameManager::Get().DestroyEntity(_uiCamera);
}

void StartScene::Init(std::shared_ptr<Entity> p1, std::shared_ptr<Entity> p2)
{
  _p1 = p1;
  _p2 = p2;

  _renderedText = GameManager::Get().CreateEntity<UITransform, TextRenderer, RenderProperties>();
  _renderedText->GetComponent<UITransform>()->anchor = UIAnchor::Center;
  _renderedText->GetComponent<TextRenderer>()->SetFont(ResourceManager::Get().GetFontWriter("fonts\\Eurostile.ttf", 36));
  _renderedText->GetComponent<TextRenderer>()->SetText("PRESS ANY BUTTON TO START");

  // set up camera
  _uiCamera = GameManager::Get().CreateEntity<Camera, Transform>();
  _uiCamera->GetComponent<Camera>()->Init(m_nativeWidth, m_nativeHeight);
  GRenderer.EstablishCamera(RenderLayer::UI, _uiCamera->GetComponent<Camera>().get());
}

void StartScene::Update(float deltaTime)
{
  UIPositionUpdateSystem::DoTick(deltaTime);
  StartSceneInputSystem::DoTick(deltaTime);
  MoveSystemCamera::DoTick(deltaTime);
}

CharacterSelectScene::~CharacterSelectScene()
{
  GameManager::Get().DestroyEntity(_portrait);
  GameManager::Get().DestroyEntity(_uiCamera);
}

void CharacterSelectScene::Init(std::shared_ptr<Entity> p1, std::shared_ptr<Entity> p2)
{
  _p1 = p1;
  _p2 = p2;

  //_portrait = GameManager::Get().CreateEntity<Transform, RenderComponent<RenderType>, RenderProperties>();
  _portrait = GameManager::Get().CreateEntity<UITransform, TextRenderer, RenderProperties>();
  _portrait->GetComponent<UITransform>()->anchor = UIAnchor::Center;
  _portrait->GetComponent<TextRenderer>()->SetFont(ResourceManager::Get().GetFontWriter("fonts\\Eurostile.ttf", 36));
  _portrait->GetComponent<TextRenderer>()->SetText("Character Select");

  // set up camera
  _uiCamera = GameManager::Get().CreateEntity<Camera, Transform>();
  _uiCamera->GetComponent<Camera>()->Init(m_nativeWidth, m_nativeHeight);
  GRenderer.EstablishCamera(RenderLayer::UI, _uiCamera->GetComponent<Camera>().get());
}

void CharacterSelectScene::Update(float deltaTime)
{
  UIPositionUpdateSystem::DoTick(deltaTime);
  CharacterSelectInputSystem::DoTick(deltaTime);
  MoveSystemCamera::DoTick(deltaTime);
}

ResultsScene::~ResultsScene()
{
  GameManager::Get().DestroyEntity(_resultText);
  GameManager::Get().DestroyEntity(_uiCamera);
}

void ResultsScene::Init(std::shared_ptr<Entity> p1, std::shared_ptr<Entity> p2)
{
  _p1 = p1;
  _p2 = p2;

  _resultText = GameManager::Get().CreateEntity<UITransform, TextRenderer, RenderProperties>();
  _resultText->GetComponent<UITransform>()->anchor = UIAnchor::Center;
  _resultText->GetComponent<TextRenderer>()->SetFont(ResourceManager::Get().GetFontWriter("fonts\\Eurostile.ttf", 36));
  
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
  _uiCamera = GameManager::Get().CreateEntity<Camera, Transform>();
  _uiCamera->GetComponent<Camera>()->Init(m_nativeWidth, m_nativeHeight);
  GRenderer.EstablishCamera(RenderLayer::UI, _uiCamera->GetComponent<Camera>().get());
}

void ResultsScene::Update(float deltaTime)
{
  UIPositionUpdateSystem::DoTick(deltaTime);
  ResultsSceneSystem::DoTick(deltaTime);
  MoveSystemCamera::DoTick(deltaTime);
}
