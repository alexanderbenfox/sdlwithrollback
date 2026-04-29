#include "Components/Input.h"
#include "Systems/InputHandlerSystem.h"
#include "DebugGUI/GUIController.h"

//______________________________________________________________________________
GameInputComponent& GameInputComponent::operator=(const GameInputComponent& other)
{
  InputHandlerSystem::AssignHandler(entityID, *this, other.GetAssignedHandler());
  return *this;
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
        InputHandlerSystem::AssignHandler(entityID, *this, InputType::Keyboard);
      else if (i == "Joystick")
        InputHandlerSystem::AssignHandler(entityID, *this, InputType::Joystick);
      else if (i == "Gamepad")
        InputHandlerSystem::AssignHandler(entityID, *this, InputType::Gamepad);
      else if (i == "DefendAll")
        InputHandlerSystem::AssignHandler(entityID, *this, InputType::DefendAll);
      else if (i == "DefendAfter")
        InputHandlerSystem::AssignHandler(entityID, *this, InputType::DefendAfter);
      else if (i == "RepeatCM")
        InputHandlerSystem::AssignHandler(entityID, *this, InputType::RepeatCM);
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
