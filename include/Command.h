#pragma once
#include "Components/GameActor.h"

//______________________________________________________________________________
class ICommand
{
public:
  virtual ~ICommand() {}
  virtual void Execute(GameActor* actor) = 0;
};

//______________________________________________________________________________
class EmptyCommand : public ICommand
{
public:
  EmptyCommand() = default;
  virtual void Execute(GameActor* actor) override
  {
    actor->HandleMovementCommand(Vector2<float>(0, 0));
  }
};

//______________________________________________________________________________
class UpCommand : public ICommand
{
public:
  UpCommand() = default;
  virtual void Execute(GameActor* actor) override
  {
    actor->HandleMovementCommand(Vector2<float>(0, -1));
  }
};

//______________________________________________________________________________
class DownCommand : public ICommand
{
public:
  DownCommand() = default;
  virtual void Execute(GameActor* actor) override
  {
    actor->HandleMovementCommand(Vector2<float>(0, 1));
  }
};

//______________________________________________________________________________
class LeftCommand : public ICommand
{
public:
  LeftCommand() = default;
  virtual void Execute(GameActor* actor) override
  {
    actor->HandleMovementCommand(Vector2<float>(-1, 0));
  }
};

//______________________________________________________________________________
class RightCommand : public ICommand
{
public:
  RightCommand() = default;
  virtual void Execute(GameActor* actor) override
  {
    actor->HandleMovementCommand(Vector2<float>(1, 0));
  }
};

//______________________________________________________________________________
class JabCommand : public ICommand
{
public:
  JabCommand() = default;
  virtual void Execute(GameActor* actor) override
  {
    actor->HandleJabButtonCommand();
  }
};
