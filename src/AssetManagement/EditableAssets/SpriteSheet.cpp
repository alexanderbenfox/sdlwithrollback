#include "SpriteSheet.h"
#include "Managers/ResourceManager.h"

//______________________________________________________________________________
SpriteSheet::SpriteSheet(const char* src, int rows, int columns) : src(src), rows(rows), columns(columns),
sheetSize(0, 0), frameSize(0, 0)
{
  GenerateSheetInfo();
}

//______________________________________________________________________________
void SpriteSheet::GenerateSheetInfo()
{
  sheetSize = ResourceManager::Get().GetTextureWidthAndHeight(src);
  frameSize = Vector2<int>(sheetSize.x / columns, sheetSize.y / rows);
}
