#pragma once
#include "Globals.h"

class GameInputComponent;
enum class InputType : int;

namespace InputHandlerSystem
{
  //! Assign an input handler type to a GameInputComponent, adding/removing AIComponent as needed
  void AssignHandler(EntityID entity, GameInputComponent& input, InputType type);
}
