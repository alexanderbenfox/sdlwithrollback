#pragma once
#include "Components/InputHandlers/IInputHandler.h"
#include <SDL2/SDL_gamecontroller.h>

//______________________________________________________________________________
//!
class GamepadInputHandler : public IInputHandler
{
public:
  //!
  GamepadInputHandler();
  //!
  ~GamepadInputHandler();
  //!
  virtual InputBuffer const& CollectInputState() final;

  void AssignKey(SDL_GameControllerButton keyCode, InputState action)
  {
    _config[keyCode] = action;
  }


private:
  //!
  SDL_GameController* _gameController = nullptr;
  //!
  SDL_Joystick* _joyStick;
  //!
  const int _joyStickID = 0;
  //!
  ConfigMap<SDL_GameControllerButton, InputState> _config;
  
};