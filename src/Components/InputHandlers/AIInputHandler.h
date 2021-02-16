#pragma once
#include "Components/InputHandlers/IInputHandler.h"
#include "Components/AIComponent.h"

//______________________________________________________________________________
//!
class AIInputHandler : public IInputHandler
{
public:
  //!
  AIInputHandler(InputBuffer& buffer) : _ai(nullptr), IInputHandler(buffer) {}
  //!
  ~AIInputHandler() = default;
  //!
  virtual InputState TranslateEvent(const SDL_Event&) final;
  //!
  virtual void SetInputMapKey(InputState value, SDL_Event key) override {}
  //! Override commit input so it does nothing until it gets interpretted later
  virtual void CommitInput(const InputState& input) override {}
  //! Use this to get interpretted input after it has been collected and synced if playing online
  virtual InputBuffer const& GetInterprettedInput() final;
  //! Sets the AI program (memory owned by the AIInputHandler)
  void SetAIProgram(AIComponent* comp, IAIProgram* program);

private:
  //!
  AIComponent* _ai;

};