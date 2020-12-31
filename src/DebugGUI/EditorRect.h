#pragma once
#include "AssetManagement/EditableAssets/IJsonLoadable.h"
#include "EditingCanvas.h"

class EditorPoint : public IJsonLoadable, public IEditorGeometryCanvas<double, Vector2>
{
public:
  EditorPoint() : IEditorGeometryCanvas<double, Vector2>() {}
  EditorPoint(Vector2<double> canvas) : IEditorGeometryCanvas<double, Vector2>(canvas) {}

  virtual void Load(const Json::Value& json) override;

  virtual void Write(Json::Value& json) const override;

  virtual void DisplayInEditor() override;

};

class EditorRect : public IJsonLoadable, public IEditorGeometryCanvas<double, Rect>
{
public:
  EditorRect() : IEditorGeometryCanvas<double, Rect>() {}
  EditorRect(Vector2<double> canvas) : IEditorGeometryCanvas<double, Rect>(canvas) {}

  virtual void Load(const Json::Value& json) override;

  virtual void Write(Json::Value& json) const override;

  virtual void DisplayInEditor() override;

};
