#pragma once
#include "Components/GameActor.h"
#include <SDL2/SDL.h>

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

void operator|=(InputState& the, InputState other);

void operator&=(InputState& the, InputState other);

InputState operator&(InputState a, InputState b);

InputState operator~(InputState og);

static bool HasState(const InputState& state, InputState other)
{
  return (state & other) == other;
}

//=========================================================================

class ICommand
{
public:
  virtual ~ICommand() {}
  virtual void Execute(GameActor* actor) = 0;
};

class EmptyCommand : public ICommand
{
public:
  EmptyCommand() = default;
  virtual void Execute(GameActor* actor) override
  {
    actor->HandleMovementCommand(Vector2<float>(0, 0));
  }
};

class UpCommand : public ICommand
{
public:
  UpCommand() = default;
  virtual void Execute(GameActor* actor) override
  {
    actor->HandleMovementCommand(Vector2<float>(0, -1));
  }
};

class DownCommand : public ICommand
{
public:
  DownCommand() = default;
  virtual void Execute(GameActor* actor) override
  {
    actor->HandleMovementCommand(Vector2<float>(0, 1));
  }
};

class LeftCommand : public ICommand
{
public:
  LeftCommand() = default;
  virtual void Execute(GameActor* actor) override
  {
    actor->HandleMovementCommand(Vector2<float>(-1, 0));
  }
};

class RightCommand : public ICommand
{
public:
  RightCommand() = default;
  virtual void Execute(GameActor* actor) override
  {
    actor->HandleMovementCommand(Vector2<float>(1, 0));
  }
};

//=========================================================================

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
