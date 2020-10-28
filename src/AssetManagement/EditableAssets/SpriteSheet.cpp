#include "SpriteSheet.h"
#include "Managers/ResourceManager.h"
#include "../imgui/imgui.h"

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

//______________________________________________________________________________
void SpriteSheet::Load(const Json::Value& json)
{
  if (!json["sheet_location"].isNull())
  {
    src = json["sheet_location"].asString();
  }
  if (!json["rows"].isNull())
  {
    rows = json["rows"].asInt();
  }
  if (!json["columns"].isNull())
  {
    columns = json["columns"].asInt();
  }
  if (!json["frameSize"].isNull())
  {
    frameSize = Vector2<int>(json["frameSize"]["x"].asInt(), json["frameSize"]["y"].asInt());
  }
  if (!json["sheetSize"].isNull())
  {
    sheetSize = Vector2<int>(json["sheetSize"]["x"].asInt(), json["sheetSize"]["y"].asInt());
  }

  if (frameSize == Vector2<int>::Zero || sheetSize == Vector2<int>::Zero)
  {
    GenerateSheetInfo();
  }
}

//______________________________________________________________________________
void SpriteSheet::Write(Json::Value& json) const
{
  json["sheet_location"] = (std::string)src;
  json["rows"] = rows;
  json["columns"] = columns;
  json["frameSize"]["x"] = frameSize.x;
  json["frameSize"]["y"] = frameSize.y;
  json["sheetSize"]["x"] = sheetSize.x;
  json["sheetSize"]["y"] = sheetSize.y;
}

//______________________________________________________________________________
void SpriteSheet::DisplayInEditor()
{
  src.DisplayEditable("File Path (Relative To Resource Folder)");
  ImGui::InputInt("Columns: ", &columns);
  ImGui::InputInt("Rows: ", &rows);

  if (ImGui::Button("Generate")) { GenerateSheetInfo(); }
}
