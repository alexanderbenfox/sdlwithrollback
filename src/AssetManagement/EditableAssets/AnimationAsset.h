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

  virtual void Load(const Json::Value& json) override
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

  virtual void Write(Json::Value& json) const override
  {
    json["sheet_name"] = sheetName;
    json["startFrame"] = startIndexOnSheet;
    json["totalFrames"] = frames;
    json["anchor"] = anchor == AnchorPoint::TL ? "TL" : anchor == AnchorPoint::TR ? "TR" : anchor == AnchorPoint::BL ? "BL" : "BR";
  }
};