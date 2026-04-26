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

//______________________________________________________________________________
//! Plays back an animation with startup/active/recovery phase display
//! and inline hitbox editing on the preview frame.
class ActionPreview
{
public:
  void Display(IAnimation* anim, ActionAsset& data);
  void Reset() { _frame = 0; _accumTime = 0.0f; _playing = false; _lastEvtFrame = -1; }

  // Commit the current frame's hitbox rect back to the ActionAsset (call before saving)
  void CommitHitbox();

private:
  void LoadHitboxForFrame(IAnimation* anim, ActionAsset& data);
  void CommitHitboxForFrame(IAnimation* anim, ActionAsset& data);

  bool _playing = false;
  float _accumTime = 0.0f;
  int _frame = 0;
  int _lastEvtFrame = -1;
  bool _editingHitboxes = false;

  void EnsureEventDataSize(IAnimation* anim, ActionAsset& data);

  // Hitbox editing state
  DisplayImage _preview;
  EditorRect _editRect;
  Vector2<double> _loadedSrcSize;  // source size used when loading the current frame's hitbox
  IAnimation* _boundAnim = nullptr;
  ActionAsset* _boundData = nullptr;
};

class CharacterConfiguration
{
public:
  CharacterConfiguration(const std::string& pathToResourceFolder);

  //! Called by CharacterEditor to render the tabbed editor UI
  void DisplayEditorTabs();

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
  ActionPreview _actionPreview;
  int _newActionSelection = 0;

  // State visualizer
  StateVisualizer _stateVisualizer;
};
