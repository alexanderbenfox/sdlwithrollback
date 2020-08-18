#pragma once
#include <SDL2/SDL.h>

#include "Components/DebugComponent/DebugComponent.h"
#include "Components/InputHandlers/KeyboardHandler.h"
#include "Components/InputHandlers/JoystickInputHandler.h"
#include "Components/InputHandlers/GamepadInputHandler.h"
#include "Components/InputHandlers/AIInputHandler.h"

enum class InputType : int
{
  Keyboard = 0, Joystick = 1, Gamepad = 2, DefendAll = 3, DefendAfter = 4, RepeatCM = 5
};

//______________________________________________________________________________
//!
class GameInputComponent : public IDebugComponent
{
public:
  GameInputComponent() : IDebugComponent("Input Handler") {}
  //! Assign this component to use a certain handler type
  void AssignHandler(InputType type);
  //! Handler interprets latest raw input and returns it
  InputBuffer const& QueryInput();
  //! Clears the input buffer
  void Clear();
  //! Assigns handler key to an action
  template <typename KeyType>
  void AssignActionKey(KeyType key, InputState action) {}
  //! Allows switching of input mode and key assignments
  void OnDebug() override;

protected:
  IInputHandler* _handler = nullptr;
  InputType _assignedHandler = InputType::Keyboard;

  KeyboardInputHandler _keyboard;
  JoystickInputHandler _joystick;
  GamepadInputHandler _gamepad;
  AIInputHandler _ai;

};

#ifdef _WIN32
#include <GGPO/ggponet.h>

struct GGPOInput
{
  GGPOSession* session;
  InputState** inputs;
  GGPOPlayerHandle** handles;
};

//______________________________________________________________________________
//!
class GGPOInputHandler : public IInputHandler
{
public:
  //!
  GGPOInputHandler() : IInputHandler() {}
  //!
  ~GGPOInputHandler() {}
  //!
  virtual InputBuffer const& CollectInputState() final;

private:
  std::shared_ptr<GGPOInput> _input;
};

#endif
