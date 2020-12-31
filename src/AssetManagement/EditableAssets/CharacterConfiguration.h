#pragma once
#include "Core/Utility/JsonFile.h"
#include "Core/Utility/FilePath.h"

#include "SpriteSheet.h"
#include "AnimationAsset.h"
#include "ActionAsset.h"

#include "DebugGUI/EditorRect.h"

class Animation;

class HitboxEditor
{
public:
  void OpenEditor(Animation* anim, ActionAsset& data, const std::string& spriteSheetID, const std::string& subSheetID);

private:
  void ChangeDisplay(Animation* anim, int frame, ActionAsset& data, const SpriteSheet::Section& sheet);

  void CommitRectChange(Animation* anim, int frame, ActionAsset& data, const SpriteSheet::Section& sheet);

  void ShowHitboxEditor();

  DisplayImage frameDisplay;
  EditorRect displayRect;
};

class CharacterConfiguration
{
public:
  CharacterConfiguration(const std::string& pathToResourceFolder);
  AssetLibrary<AnimationAsset>::LibType const& GetAnimationConfig() const { return _animations.GetLibrary(); }
  AssetLibrary<ActionAsset>::LibType const& GetActionConfig() const { return _actions.GetLibrary(); }

  void ReloadActionDebug(const std::string& actionName, ActionAsset& data);
  void AddCharacterDisplay();

private:
  template <typename T = IJsonLoadable>
  void LoadAssetFile(const char* file, AssetLibrary<T>& map)
  {
    FilePath path = _resourcePath;
    path.Append(file);

    JsonFile json(path.GetPath());
    map.LoadJsonData(json);
  }

  template <typename T = IJsonLoadable>
  void SaveAssetFile(const char* file, AssetLibrary<T>& map) const
  {
    FilePath path = _resourcePath;
    path.Append(file);

    JsonFile json(path.GetPath());
    map.SaveDataJson(json);
  }

  const FilePath _resourcePath;
  std::string _characterIdentifier;

  AssetLibrary<AnimationAsset> _animations;
  AssetLibrary<ActionAsset> _actions;

};
