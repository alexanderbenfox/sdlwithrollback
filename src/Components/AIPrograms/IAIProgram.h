#pragma once
#include "Components/InputHandlers/InputBuffer.h"

struct Transform;
class StateComponent;

struct IAIProgram
{
  //! Provide a function for how you want to update the logic based on this entity's state
  virtual InputState Update(const Transform*, const StateComponent*) = 0;
  //! Update based on the information from another entity. The current decision is passed in
  virtual InputState Update(const InputState&, const Transform*, const StateComponent*) = 0;
};
