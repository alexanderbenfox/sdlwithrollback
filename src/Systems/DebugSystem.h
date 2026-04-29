#pragma once
#include "Globals.h"

struct IDebugComponent;

namespace DebugSystem
{
  //! Registers an IDebugComponent's OnDebug with the GUIController
  void Register(IDebugComponent& comp, EntityID entity);
  //! Deregisters an IDebugComponent from the GUIController
  void Deregister(IDebugComponent& comp);
}
