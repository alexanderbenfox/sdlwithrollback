#pragma once
#include <SDL2/SDL.h>
#include <unordered_map>

#include "Command.h"

class ICommand;

//______________________________________________________________________________
enum class InputState : unsigned char
{
  NONE = 0x00,
  UP = 0x01,
  DOWN = 0x02,
  RIGHT = 0x04,
  LEFT = 0x08,
  BTN1 = 0x10,
  BTN2 = 0x20,
  BTN3 = 0x40,
  BTN4 = 0x80
};

//______________________________________________________________________________
void operator|=(InputState& the, InputState other);
//______________________________________________________________________________
void operator&=(InputState& the, InputState other);
//______________________________________________________________________________
InputState operator&(InputState a, InputState b);
//______________________________________________________________________________
InputState operator~(InputState og);
//______________________________________________________________________________
static bool HasState(const InputState& state, InputState other) { return (state & other) == other; }

//______________________________________________________________________________
//! Interface for input handlers
class IInputHandler
{
public:
  //! Gets the command based on the type of input received from the controller
  virtual ICommand* HandleInput(SDL_Event* input) = 0;

protected:
  //! Last state received by the input controller
  InputState _lastFrameState;

};

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
  virtual ICommand* HandleInput(SDL_Event* input) final;

private:
  //!
  const uint8_t* _keyStates = nullptr;
  //!
  std::unordered_map<SDL_Keycode, InputState> _config;

};

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
  virtual ICommand* HandleInput(SDL_Event* input) final;

private:
  //!
  SDL_Joystick* _gameController = nullptr;
  //!
  const int _joyStickID = 0;
  //!
  std::unordered_map<uint8_t, InputState> _config;
  
};
