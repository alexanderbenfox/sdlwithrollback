#include "AnimationAsset.h"
#include "DebugGUI/GUIController.h"

//______________________________________________________________________________
void AnimationAsset::Load(const Json::Value& json)
{
  if (!json["sheet_name"].isNull())
  {
    sheetName = json["sheet_name"].asString();
  }
  if (!json["startFrame"].isNull())
  {
    startIndexOnSheet = json["startFrame"].asInt();
  }
  if (!json["totalFrames"].isNull())
  {
    frames = json["totalFrames"].asInt();
  }
  if (!json["anchor"].isNull())
  {
    std::string anch = json["anchor"].asString();
    if (anch == "TL")
      anchor = AnchorPoint::TL;
    else if (anch == "TR")
      anchor = AnchorPoint::TR;
    else if (anch == "BL")
      anchor = AnchorPoint::BL;
    else
      anchor = AnchorPoint::BR;
  }
}

//______________________________________________________________________________
void AnimationAsset::Write(Json::Value& json) const
{
  json["sheet_name"] = sheetName;
  json["startFrame"] = startIndexOnSheet;
  json["totalFrames"] = frames;
  json["anchor"] = anchor == AnchorPoint::TL ? "TL" : anchor == AnchorPoint::TR ? "TR" : anchor == AnchorPoint::BL ? "BL" : "BR";
}

//______________________________________________________________________________
void AnimationAsset::DisplayInEditor()
{
  ImGui::BeginGroup();

  const size_t bSize = 256;
  char textBuffer[bSize];

  if (ImGui::InputText("Name Of SpriteSheet", textBuffer, bSize))
  {
    sheetName = std::string(textBuffer);
  }

  ImGui::InputInt("Start Index ", &startIndexOnSheet);
  ImGui::InputInt("Total Animation Frames: ", &frames);

  ImGui::Text("Anchor Position");
  const char* items[] = { "TL", "TR", "BL", "BR" };
  static const char* current_item = anchor == AnchorPoint::TL ? "TopLeft" : anchor == AnchorPoint::TR ? "TopRight" : anchor == AnchorPoint::BL ? "BottomLeft" : "BottomRight";
  auto func = [this](const std::string& i)
  {
    if (i == "TopLeft")
      anchor = AnchorPoint::TL;
    else if (i == "TopRight")
      anchor = AnchorPoint::TR;
    else if (i == "BottomLeft")
      anchor = AnchorPoint::BL;
    else
      anchor = AnchorPoint::BR;
  };
  DropDown::Show(current_item, items, 4, func);
  ImGui::EndGroup();
}
