#pragma once
#include "Components/InputHandlers/IInputHandler.h"
#include "Components/AIComponent.h"

//______________________________________________________________________________
//!
class AIInputHandler : public IInputHandler
{
public:
  //!
  AIInputHandler() = default;
  //!
  ~AIInputHandler() = default;
  //!
  virtual InputBuffer const& CollectInputState() final;
  //! Sets the AI program (memory owned by the AIInputHandler)
  void SetAIProgram(AIComponent* comp, IAIProgram* program);

private:
  //!
  AIComponent* _ai;

};