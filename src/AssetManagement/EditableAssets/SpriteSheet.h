#pragma once
#include "IJsonLoadable.h"
#include "Core/Math/Vector2.h"
#include "DebugGUI/EditorString.h"

struct SpriteSheet : public IJsonLoadable
{
  SpriteSheet() = default;
  SpriteSheet(const char* src, int rows, int columns);
  SpriteSheet(const char* src, int rows, int columns, bool dontLoad) : src(src), rows(rows), columns(columns) {}

  void GenerateSheetInfo();
  //!
  EditorString src;
  //!
  Vector2<int> frameSize = Vector2<int>::Zero;
  Vector2<int> sheetSize = Vector2<int>::Zero;
  //!
  int rows = 0;
  int columns = 0;

  virtual void Load(const Json::Value& json) override;
  virtual void Write(Json::Value& json) const override;
  virtual void DisplayInEditor() override;



};

