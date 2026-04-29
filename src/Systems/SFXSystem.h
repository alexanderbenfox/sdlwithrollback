#pragma once
#include "Components/SFXComponent.h"

namespace SFXSystem
{
  //! Creates child SFX entities for any SFXComponent with needsInit, then processes pending spark requests
  void DoTick();
  //! Destroys the child SFX entity — call before removing SFXComponent or when its owner is destroyed
  void Cleanup(SFXComponent& sfx);
}
