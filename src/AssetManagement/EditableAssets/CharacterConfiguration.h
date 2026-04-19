#pragma once
#include "Core/Utility/JsonFile.h"
#include "Core/Utility/FilePath.h"

#include "AnimationAsset.h"
#include "ActionAsset.h"
#include "AssetLibrary.h"

#include "DebugGUI/EditorRect.h"
#include "DebugGUI/DisplayImage.h"
#include "Core/FSM/StateVisualizer.h"

class IAnimation;

class HitboxEditor
{
public:
  void OpenEditor(IAnimation* anim, ActionAsset& data);

private:
  void ChangeDisplay(IAnimation* anim, int frame, ActionAsset& data);
  void CommitRectChange(IAnimation* anim, int frame, ActionAsset& data);
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

  void AddCharacterDisplay();

private:
  void DisplayAnimationsTab();
  void DisplayActionsTab();
  void DisplayStatesTab();

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

  // Action editing state
  std::string _selectedAction;
  HitboxEditor _hitboxEditor;

  // State visualizer
  StateVisualizer _stateVisualizer;
};
