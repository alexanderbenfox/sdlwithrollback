#pragma once
#include "IJsonLoadable.h"
#include "Core/Math/Vector2.h"
#include "Core/Geometry2D/Rect.h"
#include "DebugGUI/EditorString.h"
#include "AssetLibrary.h"

//______________________________________________________________________________
struct SpriteSheet : public IJsonLoadable
{
  class Section
  {
  public:
    DrawRect<float> GetFrame(int frame) const;
    void ShowSpriteAtIndex(const SpriteSheet& srcSheet, int frame, int height) const;

    void ShowSpriteSheetLines(const SpriteSheet& srcSheet);
    void DisplaySpriteEditor(const SpriteSheet& srcSheet, int frame);

    void LoadJson(const Json::Value& json);
    void WriteJson(Json::Value& json) const;
    void DisplayInEditor(SpriteSheet& srcSheet);
    void ShowVariableFrames(bool switchShown);

    //!
    bool variableSizeSprites = false;

    //! Only for fixed size sprites
    Vector2<int> frameSize = Vector2<int>::Zero;
    Vector2<int> offset = Vector2<int>::Zero;
    int rows = 0;
    int columns = 0;

    //! For variable size sprites
    std::vector<DrawRect<int>> frameRects;
  private:
    void DisplaySpriteEditorInternal(const SpriteSheet& srcSheet, int frame);

  };

  static std::string SaveLocation();

  SpriteSheet() = default;
  void GenerateSheetInfo();
  //!
  Section const& GetSubSection(const std::string& name) const
  {
    if (name.empty())
      return mainSection;
    else
      return subSections.at(name);
  }

  //!
  EditorString src;
  Vector2<int> sheetSize = Vector2<int>::Zero;
  //!
  Vector2<double> renderScalingFactor = Vector2<double>(1.0, 1.0);

  Section mainSection;
  std::unordered_map<std::string, Section> subSections;

  virtual void Load(const Json::Value& json) override;
  virtual void Write(Json::Value& json) const override;
  virtual void DisplayInEditor() override;

};
