#include "GameState/Scene.h"
#include "Managers/GameManagement.h"
#include "Managers/ResourceManager.h"
#include "Managers/AnimationCollectionManager.h"

#include "Systems/UISystem.h"
#include "Systems/MoveSystem.h"
#include "Systems/MenuSystem.h"
#include "Systems/DestroyEntitiesSystem.h"

#include "Components/RenderComponent.h"
#include "Components/Transform.h"
#include "Components/Camera.h"
#include "Components/Input.h"
#include "Components/StateComponent.h"

void BasicMenuScene::Update(float deltaTime)
{
  UIPositionUpdateSystem::DoTick(deltaTime);
  // menu movement and selection
  MenuInputSystem::DoTick(deltaTime);
  // change button highlight and callback if selected
  UpdateMenuStateSystem::DoTick(deltaTime);

  MoveSystemCamera::DoTick(deltaTime);
}

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

  MenuButtonArray menu(1, 5, 0.1f);
  menu.CreateMenuOption("ONLINE", [](SDL_Event){ GameManager::Get().RequestSceneChange(SceneType::BATTLEMODE); }, Vector2<int>(0, 0));
  menu.CreateMenuOption("VERSUS", [](SDL_Event) { GameManager::Get().RequestSceneChange(SceneType::BATTLEMODE); }, Vector2<int>(0, 1));
  menu.CreateMenuOption("SOLO", [](SDL_Event) { GameManager::Get().RequestSceneChange(SceneType::BATTLEMODE); }, Vector2<int>(0, 2));
  menu.CreateMenuOption("OPTIONS", [](SDL_Event) { GameManager::Get().RequestSceneChange(SceneType::CTRLSETUP); }, Vector2<int>(0, 3));
  menu.CreateMenuOption("EXIT", [](SDL_Event) { GameManager::Get().RequestSceneChange(SceneType::BATTLEMODE); }, Vector2<int>(0, 4));

  // set up camera
  _uiCamera = GameManager::Get().CreateEntity<Camera, Transform>();
  _uiCamera->GetComponent<Camera>()->Init(m_nativeWidth, m_nativeHeight);
  GRenderer.EstablishCamera(RenderLayer::UI, _uiCamera->GetComponent<Camera>());
}

CtrlSetupScene::~CtrlSetupScene()
{
  GameManager::Get().DestroyEntity(_uiCamera);
  _p1->RemoveComponent<MenuState>();
}

void CtrlSetupScene::Init(std::shared_ptr<Entity> p1, std::shared_ptr<Entity> p2)
{
  _p1 = p1;
  _p2 = p2;

  _p1->AddComponent<MenuState>();

  GameInputComponent* inputComp = _p1->GetComponent<GameInputComponent>();
  int maxInputs = std::log2((int)InputState::BTN4);
  for (int i = 0; i <= maxInputs; i++)
  {
    InputState in = static_cast<InputState>(1 << i);
    std::string label = std::string(InputStateStrings[i + 1]) + " : " + inputComp->GetAssignedInputName(in);
    _menu.CreateMenuOption(label.c_str(), [this, in, i](SDL_Event evt)
      {
        _p1->GetComponent<GameInputComponent>()->SetActionKey(evt, in);

        std::string label = std::string(InputStateStrings[i + 1]) + " : " + _p1->GetComponent<GameInputComponent>()->GetAssignedInputName(in);
        _menu.SetLabelText(0, i, label.c_str());
      }, Vector2<int>(0, i), true);
  }
  _menu.CreateMenuOption(" OK ", [](SDL_Event) { GameManager::Get().RequestSceneChange(SceneType::START); }, Vector2<int>(0, 8));

  // set up camera
  _uiCamera = GameManager::Get().CreateEntity<Camera, Transform>();
  _uiCamera->GetComponent<Camera>()->Init(m_nativeWidth, m_nativeHeight);
  GRenderer.EstablishCamera(RenderLayer::UI, _uiCamera->GetComponent<Camera>());
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
  menu.CreateMenuOption("TRAINING", [](SDL_Event) { GameManager::Get().SetBattleType(BattleType::Training); GameManager::Get().RequestSceneChange(SceneType::CSELECT); }, Vector2<int>(0, 0));
  menu.CreateMenuOption("FIRST TO 2", [](SDL_Event){ GameManager::Get().SetBattleType(BattleType::BestOf3); GameManager::Get().RequestSceneChange(SceneType::CSELECT); }, Vector2<int>(1, 0));
  menu.CreateMenuOption("FIRST TO 3", [](SDL_Event){ GameManager::Get().SetBattleType(BattleType::BestOf5); GameManager::Get().RequestSceneChange(SceneType::CSELECT); }, Vector2<int>(0, 1));
  menu.CreateMenuOption("BACK", [](SDL_Event){ GameManager::Get().RequestSceneChange(SceneType::START); }, Vector2<int>(1, 1));

  // set up camera
  _uiCamera = GameManager::Get().CreateEntity<Camera, Transform>();
  _uiCamera->GetComponent<Camera>()->Init(m_nativeWidth, m_nativeHeight);
  GRenderer.EstablishCamera(RenderLayer::UI, _uiCamera->GetComponent<Camera>());
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

  _headerLabel = GameManager::Get().CreateEntity<TextRenderer, RenderProperties, UITransform, DestroyOnSceneEnd>();
  _headerLabel->GetComponent<TextRenderer>()->SetFont(ResourceManager::Get().GetFontWriter("fonts\\Eurostile.ttf", 36));
  const Vector2<float> headerSize = _headerLabel->GetComponent<TextRenderer>()->SetText("Player 1 Choose Character!", TextAlignment::Centered);
  _headerLabel->GetComponent<UITransform>()->anchor = UIAnchor::TL;
  //const Vector2<float> headerSize(100, 100);
  const Vector2<float> headerLocation = Vector2<float>((float)m_nativeWidth / 2.0f, (float)m_nativeHeight / 14.0f);
  _headerLabel->GetComponent<UITransform>()->position = headerLocation;
  _headerLabel->GetComponent<UITransform>()->rect = Rect<float>(headerLocation, headerLocation + headerSize);

  int nCharacters = GAnimArchive.GetNCharacter();
  MenuButtonArray menu(nCharacters, 1, 0.2f);
  auto characterString = GAnimArchive.GetCharacters();
  for (int i = 0; i < characterString.size(); i++)
  {
    menu.CreateMenuOption(characterString[i].c_str(), 
      [characterString, i, this](SDL_Event)
      {
        // for now just do the same cause im lazy
        if (!_firstCharacter)
        {
          _p1->AddComponent<SelectedCharacterComponent>();
          _p1->GetComponent<SelectedCharacterComponent>()->characterIdentifier = characterString[i];
          _firstCharacter = true;
          _headerLabel->GetComponent<TextRenderer>()->SetText("Player 2 Choose Character!", TextAlignment::Centered);
        }
        else if (!_secondCharacter)
        {
          _p2->AddComponent<SelectedCharacterComponent>();
          _p2->GetComponent<SelectedCharacterComponent>()->characterIdentifier = characterString[i];
          _secondCharacter = true;
        }

        if (_firstCharacter && _secondCharacter)
          GameManager::Get().RequestSceneChange(SceneType::MATCH);
      }
    , Vector2<int>(i, 0));
  }
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
  menu.CreateMenuOption("OK", [](SDL_Event){ GameManager::Get().RequestSceneChange(SceneType::START); }, Vector2<int>(1, 2));

  _p1->GetComponent<MenuState>()->currentFocus = Vector2<int>(1, 2);
  
  // set up camera
  _uiCamera = GameManager::Get().CreateEntity<Camera, Transform>();
  _uiCamera->GetComponent<Camera>()->Init(m_nativeWidth, m_nativeHeight);
  GRenderer.EstablishCamera(RenderLayer::UI, _uiCamera->GetComponent<Camera>());
}
