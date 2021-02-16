#pragma once
#include "Components/InputHandlers/IInputHandler.h"
#include "Core/Utility/ConfigMap.h"
#include <SDL2/SDL_gamecontroller.h>

//______________________________________________________________________________
//!
class GamepadInputHandler : public IInputHandler
{
public:
  //!
  GamepadInputHandler(InputBuffer& buffer);
  //!
  ~GamepadInputHandler();
  //!
  virtual InputState TranslateEvent(const SDL_Event&) final;

  virtual void SetInputMapKey(InputState value, SDL_Event key) final;

  void AssignKey(SDL_GameControllerButton keyCode, InputState action)
  {
    _config[keyCode] = action;
  }


private:
  //!
  SDL_GameController* _gameController = nullptr;
  //!
  //SDL_Joystick* _joyStick;
  //!
  ConfigMap<SDL_GameControllerButton, InputState> _config;
  
};

//______________________________________________________________________________
//!
class NetworkInputHandler : public IInputHandler
{
public:
  //!
  NetworkInputHandler(InputBuffer& buffer) : IInputHandler(buffer) {}
  //!
  virtual InputState TranslateEvent(const SDL_Event&) final;
  //!
  virtual void SetInputMapKey(InputState value, SDL_Event key) override {}
};
