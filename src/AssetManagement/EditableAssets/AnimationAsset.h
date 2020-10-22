#pragma once
#include "IJsonLoadable.h"

enum class AnchorPoint
{
  TL, TR, BL, BR, Size
};

struct AnimationAsset : public IJsonLoadable
{
  AnimationAsset() = default;
  AnimationAsset(const std::string& sheetName, int startIndex, int nFrames, AnchorPoint anch) : sheetName(sheetName), startIndexOnSheet(startIndex), frames(nFrames), anchor(anch) {}
  std::string sheetName;
  int startIndexOnSheet;
  int frames;
  AnchorPoint anchor;

  virtual void Load(const Json::Value& json) override;

  virtual void Write(Json::Value& json) const override;

  virtual void DisplayInEditor() override;
};