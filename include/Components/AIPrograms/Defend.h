#pragma once
#include "Components/AIPrograms/IAIProgram.h"
#include "Components/IComponent.h"
#include "Components/StateComponent.h"

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
