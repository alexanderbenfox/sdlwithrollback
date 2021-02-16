#pragma once
#include "Components/InputHandlers/IInputHandler.h"
#include <SDL2/SDL_keycode.h>

/*static constexpr std::array<SDL_Scancode, 8> Keys
{
  {SDL_SCANCODE_W, SDL_SCANCODE_A, SDL_SCANCODE_S, SDL_SCANCODE_D, SDL_SCANCODE_U, SDL_SCANCODE_I, SDL_SCANCODE_O, SDL_SCANCODE_P}
};*/

static constexpr std::array<SDL_Scancode, 8> Keys
{
  {SDL_SCANCODE_W, SDL_SCANCODE_A, SDL_SCANCODE_S, SDL_SCANCODE_D, SDL_SCANCODE_U, SDL_SCANCODE_I, SDL_SCANCODE_O, SDL_SCANCODE_P}
};

static constexpr std::array<std::pair<SDL_Scancode, InputState>, 8> InitValuesForward
{
  {{SDL_SCANCODE_W, InputState::UP}, {SDL_SCANCODE_A, InputState::LEFT}, {SDL_SCANCODE_S, InputState::DOWN}, {SDL_SCANCODE_D, InputState::RIGHT},
  {SDL_SCANCODE_U, InputState::BTN1}, {SDL_SCANCODE_I, InputState::BTN2}, {SDL_SCANCODE_O, InputState::BTN3}, {SDL_SCANCODE_P, InputState::BTN4}}
};

static constexpr std::array<std::pair<InputState, SDL_Scancode>, 8> InitValuesBackward
{
  {{InputState::UP, SDL_SCANCODE_W}, {InputState::LEFT, SDL_SCANCODE_A}, {InputState::DOWN, SDL_SCANCODE_S}, {InputState::RIGHT, SDL_SCANCODE_D},
  {InputState::BTN1, SDL_SCANCODE_U}, {InputState::BTN2, SDL_SCANCODE_I}, {InputState::BTN3, SDL_SCANCODE_O}, {InputState::BTN4, SDL_SCANCODE_P}}
};

static constexpr auto KeyboardConfig = SizedConfigMap<SDL_Scancode, InputState, 8>(Keys, InitValuesForward, InitValuesBackward);
//static constexpr auto KeyboardConfig = SmallMap<SDL_Scancode, InputState, 8>{ { InitValuesForward } };

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

  virtual void SetInputMapKey(InputState value, SDL_Event key) final;
  //!
  virtual const char* GetInputName(InputState value) const override
  {
    return SDL_GetScancodeName(_config[value]);
  }
  //!
  void AssignKey(SDL_Scancode keyCode, InputState action)
  {
    _config.SetKeyValue(keyCode, action);
  }

private:
  //!
  const uint8_t* _keyStates = nullptr;
  //!
  //ConfigMap<SDL_Scancode, InputState> _config;
  SizedConfigMap<SDL_Scancode, InputState, 8> _config;

  // the only way this works is if the input is unconfigurable...
  //const SizedConfigMap<SDL_Scancode, InputState, 8>& _config;


};