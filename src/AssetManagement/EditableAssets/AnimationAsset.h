#pragma once
#include "IJsonLoadable.h"
#include "Core/Geometry2D/Rect.h"
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
  EditorString sheetName = "";
  EditorString subSheetName = "";
  int startIndexOnSheet = 0;
  int frames = 0;
  AnchorPoint anchor = AnchorPoint::TL;
  EditorPoint anchorPoints[(const int)AnchorPoint::Size];
  bool reverse = false;

  //! Per-frame hurtbox rects (one per sheet frame, may be empty for default)
  std::vector<Rect<double>> hurtboxes;

  virtual void Load(const Json::Value& json) override;

  virtual void Write(Json::Value& json) const override;

  virtual void DisplayInEditor() override;

  //! General utility functions used to get extrapolated information

  //! Displays anchor point editor using imgui
  void DisplayAnchorPointEditor();
  //! Gets position of anchor point relative to top left corner of first frame
  Vector2<float> GetAnchorPosition(int animationFrame) const;


private:
  DisplayImage _anchorEditBackground;
  bool _anchorEditBackgroundInit = false;
  bool _editorWindowDisplayed = false;

  std::string _anchorDropDownCurrentItem;

  // Animation preview playback state (editor-only, not serialized)
  int _previewFrame = 0;
  float _playbackAccumulator = 0.0f;
  bool _playing = false;
  bool _looping = false;

  // Hurtbox editor state (editor-only, not serialized)
  bool _editingHurtboxes = false;
  EditorRect _hurtboxEditRect;
  int _hurtboxLastFrame = -1;
  Vector2<double> _hurtboxSrcSize;

  void LoadHurtboxForFrame(int sheetFrame);
  void CommitHurtboxForFrame();

};