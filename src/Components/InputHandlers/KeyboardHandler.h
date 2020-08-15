#pragma once
#include "Components/InputHandlers/IInputHandler.h"
#include <SDL2/SDL_keycode.h>

//______________________________________________________________________________
//! Keyboard handler specification
class KeyboardInputHandler : public IInputHandler
{
public:
  //!
  KeyboardInputHandler();
  //!
  ~KeyboardInputHandler();
  //!
  virtual InputBuffer const& CollectInputState() final;
  //!
  void AssignKey(SDL_Keycode keyCode, InputState action)
  {
    _config[keyCode] = action;
  }

private:
  //!
  const uint8_t* _keyStates = nullptr;
  //!
  ConfigMap<SDL_Keycode, InputState> _config;

};