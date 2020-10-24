#pragma once
#include "Core/Utility/JsonFile.h"
#include "Core/Utility/FilePath.h"

#include "SpriteSheet.h"
#include "AnimationAsset.h"
#include "ActionAsset.h"

//#include "AssetManagement/AnimationEvent.h"

class Animation;
class AnimationCollection;

class CharacterConfiguration
{
public:
  CharacterConfiguration(const std::string& pathToResourceFolder);

  void AddToMenu();

  std::unordered_map<std::string, SpriteSheet> const& GetAssociatedSpriteSheets() const { return _spriteSheets; }
  std::unordered_map<std::string, AnimationAsset> const& GetAnimationConfig() const { return _animations; }
  std::unordered_map<std::string, ActionAsset> const& GetActionConfig() const { return _actions; }

  void CreateDebugMenuActions(AnimationCollection* collection);

private:

  template <typename T = IJsonLoadable>
  void LoadAssetFile(const char* file, std::unordered_map<std::string, T>& map)
  {
    FilePath path = _resourcePath;
    path.Append(file);

    JsonFile json(path.GetPath());
    json.LoadContentsIntoMap(map);
  }

  void DisplayFrameHitbox(const std::string& animName, Animation* animation, ActionAsset& data, int animationFrame);

  const FilePath _resourcePath;
  std::string _characterIdentifier;

  std::unordered_map<std::string, SpriteSheet> _spriteSheets;
  std::unordered_map<std::string, AnimationAsset> _animations;
  std::unordered_map<std::string, ActionAsset> _actions;

  //!......
  //std::unordered_map<std::string, EventBuilderDictionary> _actionEventLookupers;
};