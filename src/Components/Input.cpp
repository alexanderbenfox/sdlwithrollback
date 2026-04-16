#include "Components/Input.h"
#include "Components/AIPrograms/Defend.h"
#include "Core/ECS/Entity.h"

#include "Managers/GameManagement.h"
#include "DebugGUI/GUIController.h"

//______________________________________________________________________________
void GameInputComponent::AssignHandler(InputType type)
{
  _handler.reset();

  if ((int)_assignedHandler >= (int)InputType::DefendAll)
    GameManager::Get().GetEntityByID(entityID)->RemoveComponent<AIComponent>();
  _assignedHandler = type;

  AIInputHandler* ai = nullptr;

  switch (type)
  {
  case InputType::Keyboard:
    _handler = std::make_unique<KeyboardInputHandler>(_input);
    break;
  case InputType::Gamepad:
    _handler = std::make_unique<GamepadInputHandler>(_input);
    break;
  case InputType::Joystick:
    _handler = std::make_unique<JoystickInputHandler>(_input);
    break;
  case InputType::DefendAll:
    GameManager::Get().GetEntityByID(entityID)->AddComponent<AIComponent>();
    ai = new AIInputHandler(_input);
    ai->SetAIProgram(GameManager::Get().GetEntityByID(entityID)->GetComponent<AIComponent>(), new DefendAI);
    _handler.reset(ai);
    break;
  case InputType::DefendAfter:
    GameManager::Get().GetEntityByID(entityID)->AddComponent<AIComponent>();
    ai = new AIInputHandler(_input);
    ai->SetAIProgram(GameManager::Get().GetEntityByID(entityID)->GetComponent<AIComponent>(), new DefendAfter);
    _handler.reset(ai);
    break;
  case InputType::RepeatCM:
    GameManager::Get().GetEntityByID(entityID)->AddComponent<AIComponent>();
    ai = new AIInputHandler(_input);
    ai->SetAIProgram(GameManager::Get().GetEntityByID(entityID)->GetComponent<AIComponent>(), new RepeatInputAI(InputState::DOWN | InputState::BTN2));
    _handler.reset(ai);
    break;
  case InputType::NetworkCtrl:
    _handler = std::make_unique<NetworkInputHandler>(_input);
    break;
  default:
    break;
  }
}

//______________________________________________________________________________
void GameInputComponent::Clear()
{
  _handler->ClearInputBuffer();
}

//______________________________________________________________________________
void GameInputComponent::OnDebug() 
{
  std::string pName = "P" + std::to_string(entityID);
  if (ImGui::CollapsingHeader(pName.c_str()))
  {
    const char* items[] = { "Keyboard", "Joystick", "Gamepad", "DefendAll", "DefendAfter", "RepeatCM" };
    static const char* currentItem = nullptr;
    currentItem = items[(int)_assignedHandler];
    auto func = [this](const std::string& i)
    {
      if (i == "Keyboard")
        AssignHandler(InputType::Keyboard);
      else if (i == "Joystick")
        AssignHandler(InputType::Joystick);
      else if (i == "Gamepad")
        AssignHandler(InputType::Gamepad);
      else if (i == "DefendAll")
        AssignHandler(InputType::DefendAll);
      else if (i == "DefendAfter")
        AssignHandler(InputType::DefendAfter);
      else if (i == "RepeatCM")
        AssignHandler(InputType::RepeatCM);
    };
    DropDown::Show(currentItem, items, 6, func);
  }
}

//______________________________________________________________________________
template <>
void GameInputComponent::AssignActionKey<SDL_Scancode>(SDL_Scancode key, InputState action)
{
  reinterpret_cast<KeyboardInputHandler*>(_handler.get())->AssignKey(key, action);
}

//______________________________________________________________________________
template <>
void GameInputComponent::AssignActionKey<uint8_t>(uint8_t key, InputState action)
{
  reinterpret_cast<JoystickInputHandler*>(_handler.get())->AssignKey(key, action);
}

//______________________________________________________________________________
template <>
void GameInputComponent::AssignActionKey<SDL_GameControllerButton>(SDL_GameControllerButton key, InputState action)
{
  reinterpret_cast<GamepadInputHandler*>(_handler.get())->AssignKey(key, action);
}
