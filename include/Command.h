#pragma once
#include "Input.h"
class GameActor;

//______________________________________________________________________________
// this might need to get deleted....
class ICommand
{
public:
  virtual ~ICommand() {}
  virtual void Execute(GameActor* actor) = 0;
  virtual InputState GetInput() = 0;
};

//______________________________________________________________________________
class InGameCommand : public ICommand
{
public:
  InGameCommand(InputState input) : _input(input) {}
  virtual void Execute(GameActor* actor) override;
  virtual InputState GetInput() { return _input; }
private:
  InputState _input;
};