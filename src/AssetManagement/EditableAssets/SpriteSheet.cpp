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
std::string SpriteSheet::SaveLocation()
{
  return ResourceManager::Get().GetResourcePath() + "json\\general\\spritesheets.json";
}

static int showFrame = 0;

//______________________________________________________________________________
DrawRect<float> SpriteSheet::Section::GetFrame(int frame) const
{
  int x = frame % columns;
  int y = frame / columns;
  Vector2<float> pos(static_cast<float>(offset.x + x * frameSize.x), static_cast<float>(offset.y + y * frameSize.y));
  return DrawRect<float>(pos.x, pos.y, static_cast<float>(frameSize.x), static_cast<float>(frameSize.y));
}

//______________________________________________________________________________
void SpriteSheet::Section::ShowSpriteSheetLines(const SpriteSheet& srcSheet)
{
  ImGui::BeginChild("SpriteSheetLines");
  Vector2<int> ss = ResourceManager::Get().GetTextureWidthAndHeight(srcSheet.src);
  DisplayImage img(srcSheet.src, Rect<float>(0, 0, ss.x, ss.y), 1024);
  Vector2<float> windowPosition = img.Show();
  windowPosition.x -= ImGui::GetScrollX();
  windowPosition.y -= ImGui::GetScrollY();

  Vector2<float> scaler((float)img.displaySize.x / (float)ss.x, (float)img.displaySize.y / (float)ss.y);
  ImDrawList* draws = ImGui::GetWindowDrawList();
  for (int x = 0; x < columns; x++)
  {
    float xPos = offset.x + x * frameSize.x;
    xPos *= scaler.x;

    draws->AddLine(ImVec2(windowPosition.x + xPos, windowPosition.y + scaler.y * offset.y), ImVec2(windowPosition.x + xPos, windowPosition.y + scaler.y * srcSheet.sheetSize.y), IM_COL32(255, 255, 255, 255));
  }
  for (int y = 0; y < rows; y++)
  {
    float yPos = offset.y + y * frameSize.y;
    yPos *= scaler.y;

    draws->AddLine(ImVec2(windowPosition.x + scaler.x * offset.x, windowPosition.y + yPos), ImVec2(windowPosition.x + scaler.x * srcSheet.sheetSize.x, windowPosition.y + yPos), IM_COL32(255, 255, 255, 255));
  }

  if (ImGui::Button("Switch to Frame View"))
  {
    GUIController::Get().CreatePopup([this, &srcSheet]() { DisplayFrame(srcSheet, showFrame); }, []() {});
  }
  ImGui::InputInt("Show Frame", &showFrame);
  ImGui::DragInt2("Frame Size", &frameSize.x);
  ImGui::DragInt2("Sheet Start Offset", &offset.x, 1.0f, 0, 4096);
  ImGui::EndChild();
}

//______________________________________________________________________________
void SpriteSheet::Section::DisplayFrame(const SpriteSheet& srcSheet, int frame)
{
  ImGui::BeginChild("FrameDisplay");
  ImGui::InputInt("Show Frame", &showFrame);
  ImGui::DragInt2("Frame Size", &frameSize.x);
  ImGui::DragInt2("Sheet Start Offset", &offset.x, 1.0f, 0, 4096);
  auto frameRect = GetFrame(frame);
  DisplayImage img((std::string)srcSheet.src, Rect<float>(frameRect.x, frameRect.y, frameRect.x + frameRect.w, frameRect.y + frameRect.h), 512);
  Vector2<float> windowPosition = img.Show();

  if (ImGui::Button("Switch to Sheet View"))
  {
    GUIController::Get().CreatePopup([this, &srcSheet]() { ShowSpriteSheetLines(srcSheet); }, []() {});
  }
  ImGui::EndChild();
}

//______________________________________________________________________________
void SpriteSheet::Section::LoadJson(const Json::Value& json)
{
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
  if (!json["offset"].isNull())
  {
    offset = Vector2<int>(json["offset"]["x"].asInt(), json["offset"]["y"].asInt());
  }
}

//______________________________________________________________________________
void SpriteSheet::Section::WriteJson(Json::Value& json) const
{
  json["rows"] = rows;
  json["columns"] = columns;
  json["frameSize"]["x"] = frameSize.x;
  json["frameSize"]["y"] = frameSize.y;
  json["offset"]["x"] = offset.x;
  json["offset"]["y"] = offset.y;
}

//______________________________________________________________________________
void SpriteSheet::Section::DisplayInEditor(SpriteSheet& srcSheet)
{
  ImGui::DragInt2("Sheet Start Offset", &offset.x, 1.0f, 0, 4096);
  ImGui::InputInt("Columns: ", &columns);
  ImGui::InputInt("Rows: ", &rows);

  {
    if (ImGui::Button("Generate Frame Size"))
    {
      frameSize = Vector2<int>(srcSheet.sheetSize.x / columns, srcSheet.sheetSize.y / rows);
    }
  }

  ImGui::InputInt2("Frame Size", &frameSize.x);

  if (ImGui::Button("Show Sheet Lines"))
  {
    GUIController::Get().CreatePopup([this, &srcSheet]() { ShowSpriteSheetLines(srcSheet); }, []() {});
  }

  ImGui::InputInt("Show Frame", &showFrame);
  ImGui::SameLine();
  if (ImGui::Button("Show"))
  {
    GUIController::Get().CreatePopup([this, &srcSheet]() { DisplayFrame(srcSheet, showFrame); }, []() {});
  }
}

//______________________________________________________________________________
void SpriteSheet::GenerateSheetInfo()
{
  sheetSize = ResourceManager::Get().GetTextureWidthAndHeight(src);
  if(mainSection.columns > 0 && mainSection.rows > 0)
    mainSection.frameSize = Vector2<int>(sheetSize.x / mainSection.columns, sheetSize.y / mainSection.rows);
}

//______________________________________________________________________________
void SpriteSheet::Load(const Json::Value& json)
{
  if (!json["sheet_location"].isNull())
  {
    src = json["sheet_location"].asString();
  }
  if (!json["sheetSize"].isNull())
  {
    sheetSize = Vector2<int>(json["sheetSize"]["x"].asInt(), json["sheetSize"]["y"].asInt());
  }
  if (!json["renderScalingFactor"].isNull())
  {
    renderScalingFactor = Vector2<double>(json["renderScalingFactor"]["x"].asDouble(), json["renderScalingFactor"]["y"].asDouble());
  }

  // load main section if it exists
  mainSection.LoadJson(json);

  if (!json["SubSections"].isNull())
  {
    for (const auto& section : json["SubSections"].getMemberNames())
    {
      subSections[section].LoadJson(json["SubSections"][section]);
    }
  }

  if (sheetSize.x == 0 || sheetSize.y == 0)
    GenerateSheetInfo();
}

//______________________________________________________________________________
void SpriteSheet::Write(Json::Value& json) const
{
  json["sheet_location"] = (std::string)src;
  json["sheetSize"]["x"] = sheetSize.x;
  json["sheetSize"]["y"] = sheetSize.y;
  json["renderScalingFactor"]["x"] = renderScalingFactor.x;
  json["renderScalingFactor"]["y"] = renderScalingFactor.y;

  // write out the sections
  mainSection.WriteJson(json);
  for (const auto& section : subSections)
  {
    section.second.WriteJson(json["SubSections"][section.first]);
  }
}

//______________________________________________________________________________
void SpriteSheet::DisplayInEditor()
{
  src.DisplayEditable("File Path (Relative To Resource Folder)");

  {
    ImGui::InputInt2("Sheet Size", &sheetSize.x);
    ImGui::SameLine();
    if (ImGui::Button("Generate")) { GenerateSheetInfo(); }
  }

  ImGui::InputDouble("Render Scaling Factor X", &renderScalingFactor.x);
  ImGui::InputDouble("Render Scaling Factor Y", &renderScalingFactor.y);

  if (ImGui::CollapsingHeader("Main Sprite Sheet Segment"))
  {
    mainSection.DisplayInEditor(*this);
    if (ImGui::Button("Add As Subsection"))
    {
      static EditorString subsectionString;
      GUIController::Get().CreatePopup(
        []()
        {
          subsectionString.DisplayEditable("Subsection Name");
        },
        [this]()
        {
          subSections[(std::string)subsectionString] = mainSection;
          subsectionString.clear();
        });
    }
  }
    

  if (!subSections.empty())
  {
    if (ImGui::CollapsingHeader("Sub Sections"))
    {
      for (auto& section : subSections)
      {
        if(ImGui::CollapsingHeader(section.first.c_str()))
          section.second.DisplayInEditor(*this);
      }
    }
  }
}
