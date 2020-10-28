#pragma once
#include "Core/Math/Vector2.h"
#include "Core/Geometry2D/Rect.h"

#include <unordered_map>
#include "AssetManagement/EditableAssets/ActionAsset.h"
#include "AssetManagement/EditableAssets/SpriteSheet.h"
#include "AssetManagement/EditableAssets/ActionAsset.h"

struct ActionEditor
{
  //! only call this if ImGui::GetIO().MouseClicked[0] returns true and the click is inside the extents of the display area
  static void EditHitboxExtentsInDisplay(Vector2<double> displaySize, Vector2<double> srcSize, Vector2<double> clickPos, Rect<double>& hitbox);

};

class CharacterEditor
{
public:
  static CharacterEditor& Get()
  {
    static CharacterEditor instance;
    return instance;
  }

  void AddCreateNewCharacterButton();

private:
  CharacterEditor() = default;

};

