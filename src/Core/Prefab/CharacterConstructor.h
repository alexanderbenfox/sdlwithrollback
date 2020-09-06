#pragma once
#include "Core/ECS/Entity.h"

//______________________________________________________________________________
class CharacterConstructor
{
public:
  //! sets up character's spatial components - must set the TEAM of player afterwards
  static void InitSpatialComponents(std::shared_ptr<Entity> player, Vector2<float> position);
  //! sets up ui components - must set the SCREEN OFFSET and ANCHOR of the uiAnchor afterwards
  static std::shared_ptr<Entity> InitUIComponents(std::shared_ptr<Entity> player);

};
