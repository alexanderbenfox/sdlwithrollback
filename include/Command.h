#pragma once
#include "Input.h"
class GameActor;

//______________________________________________________________________________
class ICommand
{
public:
  virtual ~ICommand() {}
  virtual void Execute(GameActor* actor) = 0;
};

//______________________________________________________________________________
class InGameCommand : public ICommand
{
public:
  InGameCommand(InputState input) : _input(input) {}
  virtual void Execute(GameActor* actor) override;
private:
  InputState _input;
};