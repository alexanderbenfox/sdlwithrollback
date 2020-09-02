#pragma once
#include "Core/ECS/IComponent.h"
#include "Components/StateComponent.h"
#include "Components/AIPrograms/IAIProgram.h"

class ActionTimer;

//! Program with no state
struct DefendAI : IAIProgram
{
  virtual InputState Update(const Transform* t, const StateComponent* s) override;
  virtual InputState Update(const InputState& input, const Transform* t, const StateComponent* s) override;
};

//! Program with more state dependence
struct DefendAfter : DefendAI
{
  DefendAfter();
  ~DefendAfter();
  virtual InputState Update(const Transform* t, const StateComponent* s) override;
  bool wasHit = false;
  bool isDefending = false;
  std::shared_ptr<ActionTimer> resetTimer;
  std::shared_ptr<Entity> timerEntity;
  ActionState lastDefendingState = ActionState::NONE;

};

struct RepeatInputAI : IAIProgram
{
  RepeatInputAI(InputState toRepeat) : input(toRepeat) {}
  virtual InputState Update(const Transform* t, const StateComponent* s) override;
  virtual InputState Update(const InputState& input, const Transform* t, const StateComponent* s) override;

  InputState input;
};
