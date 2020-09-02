#pragma once
#include "Core/ECS/IComponent.h"
#include "Components/InputHandlers/InputBuffer.h"


//! Character type identifier components??
// Works with the CancelOnNormal component

// allows character configurations to have target combos or cancel normals into other attacks
struct AttackLinkMap : public IComponent
{
  //! Defines which normals can be cancelled into each other
  std::unordered_map<ActionState, InputState> links;

};
