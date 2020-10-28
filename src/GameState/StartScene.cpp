#include "GameState/Scene.h"
#include "Managers/GameManagement.h"
#include "Managers/ResourceManager.h"

#include "Systems/UISystem.h"
#include "Systems/MoveSystem.h"
#include "Systems/MenuSystem.h"
#include "Systems/DestroyEntitiesSystem.h"

#include "Components/RenderComponent.h"
#include "Components/Transform.h"
#include "Components/Camera.h"
#include "Components/Input.h"
#include "Components/StateComponent.h"

#include "Core/Prefab/MenuButtonArray.h"

StartScene::~StartScene()
{
  GameManager::Get().DestroyEntity(_uiCamera);
  _p1->RemoveComponent<MenuState>();
}

void StartScene::Init(std::shared_ptr<Entity> p1, std::shared_ptr<Entity> p2)
{
  _p1 = p1;
  _p2 = p2;

  _p1->AddComponent<MenuState>();

  MenuButtonArray menu(1, 1, 0.3f);
  menu.CreateMenuOption("PRESS BTN1 TO START", [](){ GameManager::Get().RequestSceneChange(SceneType::BATTLEMODE); }, Vector2<int>(0, 0));

  // set up camera
  _uiCamera = GameManager::Get().CreateEntity<Camera, Transform>();
  _uiCamera->GetComponent<Camera>()->Init(m_nativeWidth, m_nativeHeight);
  GRenderer.EstablishCamera(RenderLayer::UI, _uiCamera->GetComponent<Camera>());
}

void StartScene::Update(float deltaTime)
{
  UIPositionUpdateSystem::DoTick(deltaTime);
    // menu movement and selection
  MenuInputSystem::DoTick(deltaTime);
  // change button highlight and callback if selected
  UpdateMenuStateSystem::DoTick(deltaTime);

  MoveSystemCamera::DoTick(deltaTime);
}

BattleModeSelect::~BattleModeSelect()
{
  GameManager::Get().DestroyEntity(_uiCamera);
  _p1->RemoveComponent<MenuState>();
}

void BattleModeSelect::Init(std::shared_ptr<Entity> p1, std::shared_ptr<Entity> p2)
{
  _p1 = p1;
  _p2 = p2;

  _p1->AddComponent<MenuState>();

  MenuButtonArray menu(2, 2, 0.2f);
  menu.CreateMenuOption("TRAINING", []() { GameManager::Get().SetBattleType(BattleType::Training); GameManager::Get().RequestSceneChange(SceneType::CSELECT); }, Vector2<int>(0, 0));
  menu.CreateMenuOption("FIRST TO 2", [](){ GameManager::Get().SetBattleType(BattleType::BestOf3); GameManager::Get().RequestSceneChange(SceneType::CSELECT); }, Vector2<int>(1, 0));
  menu.CreateMenuOption("FIRST TO 3", [](){ GameManager::Get().SetBattleType(BattleType::BestOf5); GameManager::Get().RequestSceneChange(SceneType::CSELECT); }, Vector2<int>(0, 1));
  menu.CreateMenuOption("Press To Go Back", [](){ GameManager::Get().RequestSceneChange(SceneType::START); }, Vector2<int>(1, 1));

  // set up camera
  _uiCamera = GameManager::Get().CreateEntity<Camera, Transform>();
  _uiCamera->GetComponent<Camera>()->Init(m_nativeWidth, m_nativeHeight);
  GRenderer.EstablishCamera(RenderLayer::UI, _uiCamera->GetComponent<Camera>());
}

void BattleModeSelect::Update(float deltaTime)
{
  UIPositionUpdateSystem::DoTick(deltaTime);
  // menu movement and selection
  MenuInputSystem::DoTick(deltaTime);
  // change button highlight and callback if selected
  UpdateMenuStateSystem::DoTick(deltaTime);

  MoveSystemCamera::DoTick(deltaTime);
}

CharacterSelect::~CharacterSelect()
{
  _p1->RemoveComponent<MenuState>();

  // clear input buffer before battlescene
  ComponentArray<GameInputComponent>::Get().ForEach([](GameInputComponent& source)
    {
      source.Clear();
    });
}

void CharacterSelect::Init(std::shared_ptr<Entity> p1, std::shared_ptr<Entity> p2)
{
  _p1 = p1;
  _p2 = p2;

  _p1->AddComponent<MenuState>();

  int nCharacters = GAnimArchive.GetNCharacter();
  MenuButtonArray menu(nCharacters, 1, 0.2f);
  auto characterString = GAnimArchive.GetCharacters();
  for (int i = 0; i < characterString.size(); i++)
  {
    menu.CreateMenuOption(characterString[i].c_str(), 
      [characterString, i, this]()
      {
        // for now just do the same cause im lazy
        _p1->AddComponent<SelectedCharacterComponent>();
        _p1->GetComponent<SelectedCharacterComponent>()->characterIdentifier = characterString[i];
        _p2->AddComponent<SelectedCharacterComponent>();
        _p2->GetComponent<SelectedCharacterComponent>()->characterIdentifier = characterString[i];
        GameManager::Get().RequestSceneChange(SceneType::MATCH);
      }
    , Vector2<int>(i, 0));
  }
}

void CharacterSelect::Update(float deltaTime)
{
  UIPositionUpdateSystem::DoTick(deltaTime);
  // menu movement and selection
  MenuInputSystem::DoTick(deltaTime);
  // change button highlight and callback if selected
  UpdateMenuStateSystem::DoTick(deltaTime);

  MoveSystemCamera::DoTick(deltaTime);
}

ResultsScene::~ResultsScene()
{
  GameManager::Get().DestroyEntity(_resultText);
  GameManager::Get().DestroyEntity(_uiCamera);
  _p1->RemoveComponent<MenuState>();
}

void ResultsScene::Init(std::shared_ptr<Entity> p1, std::shared_ptr<Entity> p2)
{
  _p1 = p1;
  _p2 = p2;

  _p1->AddComponent<MenuState>();

  _resultText = GameManager::Get().CreateEntity<UITransform, TextRenderer, RenderProperties>();
  _resultText->GetComponent<UITransform>()->anchor = UIAnchor::Center;
  _resultText->GetComponent<TextRenderer>()->SetFont(ResourceManager::Get().GetFontWriter("fonts\\Eurostile.ttf", 36));
  
  if(_p1->GetComponent<LoserComponent>())
  {
    _resultText->GetComponent<TextRenderer>()->SetText("PLAYER 2 WINS", TextAlignment::Centered);
    _p1->RemoveComponent<LoserComponent>();
  }
  else if(_p2->GetComponent<LoserComponent>())
  {
    _resultText->GetComponent<TextRenderer>()->SetText("PLAYER 1 WINS", TextAlignment::Centered);
    _p2->RemoveComponent<LoserComponent>();
  }
  else
  {
    _resultText->GetComponent<TextRenderer>()->SetText("NOBODY LOST. SOMETHING WENT HORRIBLY WRONG.", TextAlignment::Centered);
  }

  MenuButtonArray menu(3, 3, 0.1f);
  menu.CreateMenuOption("OK", [](){ GameManager::Get().RequestSceneChange(SceneType::START); }, Vector2<int>(1, 2));

  _p1->GetComponent<MenuState>()->currentFocus = Vector2<int>(1, 2);
  
  // set up camera
  _uiCamera = GameManager::Get().CreateEntity<Camera, Transform>();
  _uiCamera->GetComponent<Camera>()->Init(m_nativeWidth, m_nativeHeight);
  GRenderer.EstablishCamera(RenderLayer::UI, _uiCamera->GetComponent<Camera>());
}

void ResultsScene::Update(float deltaTime)
{
  UIPositionUpdateSystem::DoTick(deltaTime);
    // menu movement and selection
  MenuInputSystem::DoTick(deltaTime);
  // change button highlight and callback if selected
  UpdateMenuStateSystem::DoTick(deltaTime);

  MoveSystemCamera::DoTick(deltaTime);
}
