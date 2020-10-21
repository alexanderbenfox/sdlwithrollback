#pragma once
#include "IJsonLoadable.h"
#include "Core/Math/Vector2.h"

struct SpriteSheet : public IJsonLoadable
{
  SpriteSheet() = default;
  SpriteSheet(const char* src, int rows, int columns);
  SpriteSheet(const char* src, int rows, int columns, bool dontLoad) : src(src), rows(rows), columns(columns) {}

  void GenerateSheetInfo();
  //!
  std::string src;
  //!
  Vector2<int> frameSize;
  Vector2<int> sheetSize;
  //!
  int rows, columns;

  virtual void Load(const Json::Value& json) override
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

  virtual void Write(Json::Value& json) const override
  {
    json["sheet_location"] = src;
    json["rows"] = rows;
    json["columns"] = columns;
    json["frameSize"]["x"] = frameSize.x;
    json["frameSize"]["y"] = frameSize.y;
    json["sheetSize"]["x"] = sheetSize.x;
    json["sheetSize"]["y"] = sheetSize.y;
  }
};
