#include "AnimationAsset.h"
#include "DebugGUI/GUIController.h"
#include "DebugGUI/DisplayImage.h"

#include "AssetManagement/Animation.h"
#include "AssetLibrary.h"

// this is for the find anchor
#include "Managers/GameManagement.h"

//______________________________________________________________________________
namespace std
{
  string to_string(AnchorPoint value)
  {
    switch (value)
    {
    case AnchorPoint::BL: return "BL";
    case AnchorPoint::TL: return "TL";
    case AnchorPoint::BR: return "BR";
    case AnchorPoint::TR: return "TL";
    case AnchorPoint::Center: return "Center";
    default: return "NaN";
    }
  }
}

//______________________________________________________________________________
AnchorPoint APFromString(const std::string i)
{
  if (i == "TL")
    return AnchorPoint::TL;
  else if (i == "TR")
    return AnchorPoint::TR;
  else if (i == "BL")
    return AnchorPoint::BL;
  else if (i == "BR")
    return AnchorPoint::BR;
  else
    return AnchorPoint::Center;
}

//______________________________________________________________________________
Vector2<float> CalculateRenderOffset(AnchorPoint anchor, const Vector2<float>& textureRenderOffset, const Vector2<float>& rectTransform)
{
  Vector2<float> offset = textureRenderOffset;

  if (anchor == AnchorPoint::TL) {}
  else if (anchor == AnchorPoint::BL)
  {
    offset.y -= rectTransform.y;
  }
  else if (anchor == AnchorPoint::TR)
  {
    offset.x -= rectTransform.x;
  }
  else if (anchor == AnchorPoint::BR)
  {
    offset -= rectTransform;
  }
  else if (anchor == AnchorPoint::Center)
  {
    offset -= (rectTransform / 2.0f);
  }

  return -rectTransform / 2.0f - offset;
}

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
    anchor = APFromString(json["anchor"].asString());
  }
  if (!json["anchorPoints"].isNull())
  {
    auto ap = json["anchorPoints"];
    anchorPoints[(int)AnchorPoint::TL].Load(ap["TL"]);
    anchorPoints[(int)AnchorPoint::TR].Load(ap["TR"]);
    anchorPoints[(int)AnchorPoint::BL].Load(ap["BL"]);
    anchorPoints[(int)AnchorPoint::BR].Load(ap["BR"]);
    anchorPoints[(int)AnchorPoint::Center].Load(ap["Center"]);
  }
}

//______________________________________________________________________________
void AnimationAsset::Write(Json::Value& json) const
{
  json["sheet_name"] = (std::string)sheetName;
  json["startFrame"] = startIndexOnSheet;
  json["totalFrames"] = frames;
  json["anchor"] = std::to_string(anchor);

  auto& ap = json["anchorPoints"] = Json::Value(Json::objectValue);

  anchorPoints[(int)AnchorPoint::TL].Write(ap["TL"]);
  anchorPoints[(int)AnchorPoint::TR].Write(ap["TR"]);
  anchorPoints[(int)AnchorPoint::BL].Write(ap["BL"]);
  anchorPoints[(int)AnchorPoint::BR].Write(ap["BR"]);
  anchorPoints[(int)AnchorPoint::Center].Write(ap["Center"]);
}

//______________________________________________________________________________
void AnimationAsset::DisplayInEditor()
{
  ImGui::BeginGroup();
  sheetName.DisplayEditable("Name of SpriteSheet");
  ImGui::InputInt("Start Index ", &startIndexOnSheet);
  ImGui::InputInt("Total Animation Frames: ", &frames);

  ImGui::Text("Anchor Position");
  const char* items[] = { "TL", "TR", "BL", "BR", "Center" };
  _anchorDropDownCurrentItem = std::to_string(anchor);
  auto func = [this](const std::string& i) { anchor = APFromString(i); };
  DropDown::Show(_anchorDropDownCurrentItem.c_str(), items, 5, func);

  DisplayAnchorPointEditor();

  ImGui::EndGroup();
}

//______________________________________________________________________________
void AnimationAsset::DisplayAnchorPointEditor()
{
  if (_editorWindowDisplayed)
  {
    ImGui::Begin("Anchor Point Editor");
    if (!_anchorEditBackgroundInit)
    {
      const SpriteSheet& animSpriteSheet = AssetLibrary<SpriteSheet>::Get(sheetName);
      Vector2<float> frameSize = animSpriteSheet.frameSize;

      int x = startIndexOnSheet % animSpriteSheet.columns;
      int y = startIndexOnSheet / animSpriteSheet.columns;
      Vector2<float> tlPos(x * frameSize.x, y * frameSize.y);

      _anchorEditBackground = DisplayImage(animSpriteSheet.src, Rect<float>(tlPos, tlPos + frameSize), 512);

      if (_anchorEditBackground.ptr)
      {
        for (int i = 0; i < (int)AnchorPoint::Size; i++)
        {
          anchorPoints[(int)i].SetCanvasSize(_anchorEditBackground.displaySize);
        }
        _anchorEditBackgroundInit = true;
      }
    }

    if (_anchorEditBackground.ptr)
    {
      std::string anchorPtLabel = std::to_string(anchor);
      ImGui::Text("Displaying Anchor Point = %s", anchorPtLabel.c_str());

      // display image within child
      ImGui::BeginChild("Anchor Point Editor");

      Vector2<float> position = _anchorEditBackground.Show();
      anchorPoints[(int)anchor].DisplayAtPosition(position);

      ImGui::EndChild();

      if (ImGui::Button("Auto Calculate Anchor Points"))
      {
        const SpriteSheet& animSpriteSheet = AssetLibrary<SpriteSheet>::Get(sheetName);
        Vector2<int> anchorPos = GenerateAnchorPoint(anchor, animSpriteSheet, startIndexOnSheet, false);
        anchorPoints[(int)anchor].Import(anchorPos, animSpriteSheet.frameSize);
      }
    }

    if (ImGui::Button("Close"))
    {
      _editorWindowDisplayed = false;
    }
    ImGui::End();
  }
  else
  {
    if (ImGui::Button("Edit Anchor Points"))
    {
      _editorWindowDisplayed = true;
    }
  }

}

//______________________________________________________________________________
Vector2<float> AnimationAsset::GetAnchorPosition() const
{
  const SpriteSheet& animSpriteSheet = AssetLibrary<SpriteSheet>::Get(sheetName);
  Vector2<double> const& anchPos = anchorPoints[(int)anchor].Export(animSpriteSheet.frameSize);
  return static_cast<Vector2<float>>(anchPos);
}

//______________________________________________________________________________
Vector2<int> AnimationAsset::GenerateAnchorPoint(AnchorPoint anchorType, const SpriteSheet& spriteSheet, int startIdx, bool fromFirstFrame)
{
  Resource<SDL_Texture>& sheetTexture = ResourceManager::Get().GetAsset<SDL_Texture>(spriteSheet.src);
  // Get the window format
  Uint32 windowFormat = SDL_GetWindowPixelFormat(GRenderer.GetWindow());
  std::shared_ptr<SDL_PixelFormat> format = std::shared_ptr<SDL_PixelFormat>(SDL_AllocFormat(windowFormat), SDL_FreeFormat);

  // Get the pixel data
  Uint32* upixels;
#ifdef _WIN32
  unsigned char* px = sheetTexture.GetInfo().pixels.get();
  upixels = (Uint32*)px;
#else
  upixels = (Uint32*)sheetTexture.GetInfo().pixels.get();
#endif

  auto findAnchor = [&spriteSheet, &upixels, &sheetTexture, &format](bool reverseX, bool reverseY, int startX, int startY)
  {
#ifdef _WIN32
    Uint32 transparent = sheetTexture.GetInfo().transparent;
#endif
    int strX = startX;
    int strY = startY;
    startX = reverseX ? startX - 1 : startX + 1;
    startY = reverseY ? startY - 1 : startY + 1;
    for (int yValue = startY; yValue < startY + spriteSheet.frameSize.y; yValue++)
    {
      for (int xValue = startX; xValue < startX + spriteSheet.frameSize.x; xValue++)
      {
        int y = yValue;
        if (reverseY)
          y = startY + spriteSheet.frameSize.y - (yValue - startY);
        int x = xValue;
        if (reverseX)
          x = startX + spriteSheet.frameSize.x - (xValue - startX);

        Uint32 pixel = upixels[sheetTexture.GetInfo().mWidth * y + x];
#ifdef _WIN32
        if (pixel != transparent)
#else
        Uint8 r, g, b, a;
        SDL_GetRGBA(pixel, format.get(), &r, &g, &b, &a);
        if (a == 0xFF)
#endif
          return Vector2<int>(x - strX, y - strY);
      }
    }
    return Vector2<int>(0, 0);
  };

  int startX = (startIdx % spriteSheet.columns) * spriteSheet.frameSize.x;
  int startY = (startIdx / spriteSheet.columns) * spriteSheet.frameSize.y;
  bool reverseX = anchorType == AnchorPoint::TR || anchorType == AnchorPoint::BR;
  bool reverseY = anchorType == AnchorPoint::BR || anchorType == AnchorPoint::BL;
  if (fromFirstFrame)
    return findAnchor(reverseX, reverseY, 0, 0);
  else
    return findAnchor(reverseX, reverseY, startX, startY);

}
