#include "Components/Input.h"
#include "Components/AIPrograms/Defend.h"
#include "Core/ECS/Entity.h"

#include "Managers/GameManagement.h"
#include "DebugGUI/GUIController.h"

//______________________________________________________________________________
void GameInputComponent::AssignHandler(InputType type)
{
  if((int)_assignedHandler >= (int)InputType::DefendAll)
    GameManager::Get().GetEntityByID(entityID)->RemoveComponent<AIComponent>();

  _assignedHandler = type;
  switch (type)
  {
  case InputType::Keyboard:
    _handler = &_keyboard;
    break;
  case InputType::Gamepad:
    _handler = &_gamepad;
    break;
  case InputType::Joystick:
    _handler = &_joystick;
    break;
  case InputType::DefendAll:
    GameManager::Get().GetEntityByID(entityID)->AddComponent<AIComponent>();
    _ai.SetAIProgram(GameManager::Get().GetEntityByID(entityID)->GetComponent<AIComponent>(), new DefendAI);
    _handler = &_ai;
    break;
  case InputType::DefendAfter:
    GameManager::Get().GetEntityByID(entityID)->AddComponent<AIComponent>();
    _ai.SetAIProgram(GameManager::Get().GetEntityByID(entityID)->GetComponent<AIComponent>(), new DefendAfter);
    _handler = &_ai;
  default:
    break;
  }
}

//______________________________________________________________________________
InputBuffer const& GameInputComponent::QueryInput()
{
  return _handler->CollectInputState();
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
    const char* items[] = { "Keyboard", "Joystick", "Gamepad", "DefendAll", "DefendAfter" };
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
    };
    DropDown::Show(currentItem, items, 5, func);
  }
}

//______________________________________________________________________________
template <>
void GameInputComponent::AssignActionKey<SDL_Keycode>(SDL_Keycode key, InputState action)
{
  _keyboard.AssignKey(key, action);
}

//______________________________________________________________________________
template <>
void GameInputComponent::AssignActionKey<uint8_t>(uint8_t key, InputState action)
{
  _joystick.AssignKey(key, action);
}

//______________________________________________________________________________
template <>
void GameInputComponent::AssignActionKey<SDL_GameControllerButton>(SDL_GameControllerButton key, InputState action)
{
  _gamepad.AssignKey(key, action);
}

#ifdef _WIN32
//______________________________________________________________________________
InputBuffer const& GGPOInputHandler::CollectInputState()
{
  // notify ggpo of local player's inputs
  GGPOErrorCode result = ggpo_add_local_input(_input->session, (*_input->handles)[0], &(*_input->inputs)[0], sizeof((*_input->inputs)[0]));

  // synchronize inputs
  if (GGPO_SUCCEEDED(result))
  {
    int disconnectFlags;
    result = ggpo_synchronize_input(_input->session, (*_input->inputs), sizeof(*_input->inputs), &disconnectFlags);
    if (GGPO_SUCCEEDED(result))
    {
      _inputBuffer.Push((*_input->inputs)[1]);
    }
    else
    {
      _inputBuffer.Push(InputState::NONE);
    }
  }
  else
  {
    _inputBuffer.Push(InputState::NONE);
  }
  return _inputBuffer;

}
#endif