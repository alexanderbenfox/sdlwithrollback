#pragma once
#include "Components/IComponent.h"
#include "Components/InputHandlers/InputBuffer.h"
#include "Components/Transform.h"
#include "Components/StateComponent.h"

#include "Components/AIPrograms/IAIProgram.h"

// holds data relevant to AI decision making
class AIComponent : public IComponent
{
public:
  AIComponent(std::shared_ptr<Entity> entity) : IComponent(entity) {}

  void UpdateMyState(const Transform* transform, const StateComponent* state);

  void UpdateFromOther(const Transform* transform, const StateComponent* state);

  std::unique_ptr<IAIProgram> program;

  InputState lastDecision;

};

inline void AIComponent::UpdateMyState(const Transform* transform, const StateComponent* state)
{
  lastDecision = InputState::NONE;
  if(program)
    lastDecision = program->Update(transform, state);
}

inline void AIComponent::UpdateFromOther(const Transform* transform, const StateComponent* state)
{
  if(program)
    lastDecision = program->Update(lastDecision, transform, state);
}
