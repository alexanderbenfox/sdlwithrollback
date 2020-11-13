#pragma once
#include "IJsonLoadable.h"
#include "DebugGUI/EditorString.h"
#include "DebugGUI/EditorRect.h"
#include "DebugGUI/DisplayImage.h"

#include "SpriteSheet.h"

enum class AnchorPoint
{
  TL, TR, BL, BR, Center, Size
};

namespace std
{
  string to_string(AnchorPoint value);
}

AnchorPoint APFromString(const std::string i);

Vector2<float> CalculateRenderOffset(AnchorPoint anchor, const Vector2<float>& textureRenderOffset, const Vector2<float>& rectTransform);

class AnimationAsset : public IJsonLoadable
{
public:
  AnimationAsset() = default;
  AnimationAsset(const std::string& sheetName, int startIndex, int nFrames, AnchorPoint anch) : sheetName(sheetName), startIndexOnSheet(startIndex), frames(nFrames), anchor(anch) {}
  EditorString sheetName;
  int startIndexOnSheet = 0;
  int frames = 0;
  AnchorPoint anchor = AnchorPoint::TL;
  //Vector2<int> anchorPoints[(const int)AnchorPoint::Size];
  EditorPoint anchorPoints[(const int)AnchorPoint::Size];
  bool reverse = false;

  virtual void Load(const Json::Value& json) override;

  virtual void Write(Json::Value& json) const override;

  virtual void DisplayInEditor() override;

  //! General utility functions used to get extrapolated information

  //! Displays anchor point editor using imgui
  void DisplayAnchorPointEditor();
  //! Gets position of anchor point relative to top left corner of first frame
  Vector2<float> GetAnchorPosition() const;


  //! Gets first non-transparent pixel from the top left and bottom left
  static Vector2<int> GenerateAnchorPoint(AnchorPoint anchorType, const SpriteSheet& spriteSheet, int startIdx, bool fromFirstFrame);

private:
  DisplayImage _anchorEditBackground;
  bool _anchorEditBackgroundInit = false;
  bool _editorWindowDisplayed = false;

  std::string _anchorDropDownCurrentItem;

};