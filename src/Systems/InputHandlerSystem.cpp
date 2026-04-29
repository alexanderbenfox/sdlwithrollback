#include "Systems/InputHandlerSystem.h"
#include "Components/Input.h"
#include "Components/AIPrograms/Defend.h"
#include "Managers/GameManagement.h"

//______________________________________________________________________________
void InputHandlerSystem::AssignHandler(EntityID entity, GameInputComponent& input, InputType type)
{
  input._handler.reset();

  if ((int)input._assignedHandler >= (int)InputType::DefendAll)
    GameManager::Get().GetEntityByID(entity)->RemoveComponent<AIComponent>();
  input._assignedHandler = type;

  AIInputHandler* ai = nullptr;

  switch (type)
  {
  case InputType::Keyboard:
    input._handler = std::make_unique<KeyboardInputHandler>(input._input);
    break;
  case InputType::Gamepad:
    input._handler = std::make_unique<GamepadInputHandler>(input._input);
    break;
  case InputType::Joystick:
    input._handler = std::make_unique<JoystickInputHandler>(input._input);
    break;
  case InputType::DefendAll:
    GameManager::Get().GetEntityByID(entity)->AddComponent<AIComponent>();
    ai = new AIInputHandler(input._input);
    ai->SetAIProgram(GameManager::Get().GetEntityByID(entity)->GetComponent<AIComponent>(), new DefendAI);
    input._handler.reset(ai);
    break;
  case InputType::DefendAfter:
    GameManager::Get().GetEntityByID(entity)->AddComponent<AIComponent>();
    ai = new AIInputHandler(input._input);
    ai->SetAIProgram(GameManager::Get().GetEntityByID(entity)->GetComponent<AIComponent>(), new DefendAfter);
    input._handler.reset(ai);
    break;
  case InputType::RepeatCM:
    GameManager::Get().GetEntityByID(entity)->AddComponent<AIComponent>();
    ai = new AIInputHandler(input._input);
    ai->SetAIProgram(GameManager::Get().GetEntityByID(entity)->GetComponent<AIComponent>(), new RepeatInputAI(InputState::DOWN | InputState::BTN2));
    input._handler.reset(ai);
    break;
  case InputType::NetworkCtrl:
    input._handler = std::make_unique<NetworkInputHandler>(input._input);
    break;
  default:
    break;
  }
}
