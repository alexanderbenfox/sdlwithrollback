#pragma once
#include "Components/InputHandlers/IInputHandler.h"
#include "SDL2/SDL_joystick.h"

//______________________________________________________________________________
//!
class JoystickInputHandler : public IInputHandler
{
public:
  //!
  JoystickInputHandler();
  //!
  ~JoystickInputHandler();
  //!
  virtual InputBuffer const& CollectInputState() final;

  void AssignKey(uint8_t keyCode, InputState action)
  {
    _config[keyCode] = action;
  }

private:
  //!
  SDL_Joystick* _gameController = nullptr;
  //!
  ConfigMap<uint8_t, InputState> _config;
  
};