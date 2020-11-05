#pragma once
#include "IJsonLoadable.h"
#include "DebugGUI/EditorString.h"
#include "DebugGUI/EditorRect.h"
#include "DebugGUI/DisplayImage.h"

#include "SpriteSheet.h"

enum class AnchorPoint
{
  TL, TR, BL, BR, Size
};

struct AnimationAsset : public IJsonLoadable
{
  AnimationAsset() = default;
  AnimationAsset(const std::string& sheetName, int startIndex, int nFrames, AnchorPoint anch) : sheetName(sheetName), startIndexOnSheet(startIndex), frames(nFrames), anchor(anch) {}
  EditorString sheetName;
  int startIndexOnSheet = 0;
  int frames = 0;
  AnchorPoint anchor = AnchorPoint::TL;
  //Vector2<int> anchorPoints[(const int)AnchorPoint::Size];
  EditorPoint anchorPoints[(const int)AnchorPoint::Size];

  virtual void Load(const Json::Value& json) override;

  virtual void Write(Json::Value& json) const override;

  virtual void DisplayInEditor() override;

  void DisplayAnchorPointEditor(const SpriteSheet& animSpriteSheet);

  DisplayImage anchorEditBackground;
  bool anchorEditBackgroundInit = false;

  //! Gets first non-transparent pixel from the top left and bottom left
  static Vector2<int> FindAnchorPoint(AnchorPoint anchorType, const SpriteSheet& spriteSheet, int startIdx, bool fromFirstFrame);

};