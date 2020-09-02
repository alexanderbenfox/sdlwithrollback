#pragma once
#include "Components/InputHandlers/IInputHandler.h"
#include <SDL2/SDL_keycode.h>

//______________________________________________________________________________
//! Keyboard handler specification
class KeyboardInputHandler : public IInputHandler
{
public:
  //!
  KeyboardInputHandler(InputBuffer& buffer);
  //!
  ~KeyboardInputHandler();
  //!
  virtual InputState TranslateEvent(const SDL_Event&) final;
  //!
  void AssignKey(SDL_Scancode keyCode, InputState action)
  {
    _config[keyCode] = action;
  }

private:
  //!
  const uint8_t* _keyStates = nullptr;
  //!
  //ConfigMap<SDL_Keycode, InputState> _eventConfig;
  ConfigMap<SDL_Scancode, InputState> _config;
};