#pragma once
#include "Core/Utility/JsonFile.h"
#include "Core/Utility/FilePath.h"

#include "SpriteSheet.h"
#include "AnimationAsset.h"
#include "ActionAsset.h"

#include "DebugGUI/EditorRect.h"

//#include "AssetManagement/AnimationEvent.h"

class Animation;
class AnimationCollection;

class HitboxEditor
{
public:
  void OpenEditor(Animation* anim, ActionAsset& data, const std::string& spriteSheetID);

private:
  void ChangeDisplay(Animation* anim, int frame, ActionAsset& data, const SpriteSheet& sheet);

  void CommitRectChange(Animation* anim, int frame, ActionAsset& data, const SpriteSheet& sheet);

  void ShowHitboxEditor();

  DisplayImage frameDisplay;
  EditorRect displayRect;
};

class CharacterConfiguration
{
public:
  CharacterConfiguration(const std::string& pathToResourceFolder);
  std::unordered_map<std::string, AnimationAsset> const& GetAnimationConfig() const { return _animations; }
  std::unordered_map<std::string, ActionAsset> const& GetActionConfig() const { return _actions; }

  void ReloadActionDebug(const std::string& actionName, ActionAsset& data);
  void AddCharacterDisplay();

private:
  template <typename T = IJsonLoadable>
  void LoadAssetFile(const char* file, std::unordered_map<std::string, T>& map)
  {
    FilePath path = _resourcePath;
    path.Append(file);

    JsonFile json(path.GetPath());
    json.LoadContentsIntoMap(map);
  }

  template <typename T = IJsonLoadable>
  void SaveAssetFile(const char* file, std::unordered_map<std::string, T>& map) const
  {
    FilePath path = _resourcePath;
    path.Append(file);

    JsonFile json(path.GetPath());
    json.SaveContentsIntoFile(map);
  }

  const FilePath _resourcePath;
  std::string _characterIdentifier;

  std::unordered_map<std::string, AnimationAsset> _animations;
  std::unordered_map<std::string, ActionAsset> _actions;

  //! Stuff for editor display
  EditorString newSheetName;
  SpriteSheet newSheet;
  EditorString animName;
  AnimationAsset newAnimation;
  EditorString actionName;
  ActionAsset newAction;

  //!......
  //std::unordered_map<std::string, EventBuilderDictionary> _actionEventLookupers;
};
