#include "SpriteSheet.h"
#include "Managers/ResourceManager.h"
#include "Managers/GameManagement.h"
#include "../imgui/imgui.h"
#include "AssetLibrary.h"

//______________________________________________________________________________
template <> void AssetLoaderFn::OnLoad(SpriteSheet& asset)
{
  ResourceManager::Get().GetAsset<RenderType>(asset.src);
}

//______________________________________________________________________________
template <> std::string AssetLoaderFn::SaveLocation<SpriteSheet>()
{
  return ResourceManager::Get().GetResourcePath() + "json\\general\\spritesheets.json";
}

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
void SpriteSheet::ShowSpriteSheetLines()
{
  ImGui::BeginChild("SpriteSheetLines");
  Vector2<int> ss = ResourceManager::Get().GetTextureWidthAndHeight(src);
  DisplayImage img(src, Rect<float>(0, 0, ss.x, ss.y), 1024);
  Vector2<float> windowPosition = img.Show();
  windowPosition.x -= ImGui::GetScrollX();
  windowPosition.y -= ImGui::GetScrollY();

  Vector2<float> scaler((float)img.displaySize.x / (float)ss.x, (float)img.displaySize.y / (float)ss.y);
  ImDrawList* draws = ImGui::GetWindowDrawList();
  for (int x = 0; x < columns; x++)
  {
    float xPos = offset.x + x * frameSize.x;
    xPos *= scaler.x;

    draws->AddLine(ImVec2(windowPosition.x + xPos, windowPosition.y + scaler.y * offset.y), ImVec2(windowPosition.x + xPos, windowPosition.y + scaler.y * sheetSize.y), IM_COL32(255, 255, 255, 255));
  }
  for (int y = 0; y < rows; y++)
  {
    float yPos = offset.y + y * frameSize.y;
    yPos *= scaler.y;

    draws->AddLine(ImVec2(windowPosition.x + scaler.x * offset.x, windowPosition.y + yPos), ImVec2(windowPosition.x + scaler.x * sheetSize.x, windowPosition.y + yPos), IM_COL32(255, 255, 255, 255));
  }
  ImGui::EndChild();
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
  if (!json["offset"].isNull())
  {
    offset = Vector2<int>(json["offset"]["x"].asInt(), json["offset"]["y"].asInt());
  }
  if (!json["renderScalingFactor"].isNull())
  {
    renderScalingFactor = Vector2<double>(json["renderScalingFactor"]["x"].asDouble(), json["renderScalingFactor"]["y"].asDouble());
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
  json["offset"]["x"] = offset.x;
  json["offset"]["y"] = offset.y;
  json["renderScalingFactor"]["x"] = renderScalingFactor.x;
  json["renderScalingFactor"]["y"] = renderScalingFactor.y;
}

//______________________________________________________________________________
void SpriteSheet::DisplayInEditor()
{
  src.DisplayEditable("File Path (Relative To Resource Folder)");
  ImGui::InputInt2("Sheet Start Offset", &offset.x);
  ImGui::InputInt("Columns: ", &columns);
  ImGui::InputInt("Rows: ", &rows);

  {
    ImGui::InputInt2("Sheet Size", &sheetSize.x);
    ImGui::SameLine();
    if (ImGui::Button("Generate Frame Size"))
    {
      frameSize = Vector2<int>(sheetSize.x / columns, sheetSize.y / rows);
    }
  }

  ImGui::InputInt2("Frame Size", &frameSize.x);

  ImGui::InputDouble("Render Scaling Factor X", &renderScalingFactor.x);
  ImGui::InputDouble("Render Scaling Factor Y", &renderScalingFactor.y);

  if (ImGui::Button("Generate")) { GenerateSheetInfo(); }
  if (ImGui::Button("Show Sheet Lines"))
  {
    GUIController::Get().CreatePopup([this]() { ShowSpriteSheetLines(); }, []() {});
  }
}
