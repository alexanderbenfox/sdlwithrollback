#pragma once
#include "GameActor.h"
#include <SDL2/SDL.h>

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

class Move : public ICommand
{
public:
  Move(Vector2<int> direction) : _direction(direction) {}
  virtual void Execute(GameActor& actor) override
  {}
protected:
  Vector2<int> _direction;
};

class IInputHandler
{
public:
  virtual ICommand* HandleInput(SDL_Event* input) = 0;
};

class KeyboardInputHandler : public IInputHandler
{
public:
  KeyboardInputHandler();
  ~KeyboardInputHandler();
  virtual ICommand* HandleInput(SDL_Event* input) final;

private:
  const uint8_t* _keyStates = nullptr;
};

class JoystickInputHandler : public IInputHandler
{
public:
  JoystickInputHandler();
  ~JoystickInputHandler();
  virtual ICommand* HandleInput(SDL_Event* input) final;

private:
};