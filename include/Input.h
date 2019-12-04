#pragma once
#include "GameActor.h"
#include <SDL2/SDL.h>

enum class InputState : uint16_t
{
  NONE = 0x00,
  UP = 0x01,
  DOWN = 0x02,
  RIGHT = 0x04,
  LEFT = 0x08,
  BTN1 = 0x0f,
  BTN2 = 0x11,
  BTN3 = 0x12
};

class ICommand
{
public:
  virtual ~ICommand() {}
  virtual void Execute(GameActor& actor) = 0;
};

class EmptyCommand : public ICommand
{
public:
  EmptyCommand() = default;
  virtual void Execute(GameActor& actor) override {}
};

class IInputHandler
{
public:
  virtual ICommand* HandleInput(SDL_Event* input) = 0;
protected:
  InputState _lastFrameState;
};

class KeyboardInputHandler : public IInputHandler
{
public:
  KeyboardInputHandler();
  ~KeyboardInputHandler();
  virtual ICommand* HandleInput(SDL_Event* input) final;

private:
  const uint8_t* _keyStates = nullptr;
  std::unordered_map<SDL_Keycode, InputState> _config;

};

class JoystickInputHandler : public IInputHandler
{
public:
  JoystickInputHandler();
  ~JoystickInputHandler();
  virtual ICommand* HandleInput(SDL_Event* input) final;

private:
  //
  SDL_Joystick* _gameController = nullptr;
  //
  const int _joyStickID = 0;
  //
  std::unordered_map<uint8_t, InputState> _config;
  
};
