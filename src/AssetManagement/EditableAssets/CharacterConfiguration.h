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
  CharacterConfiguration(const std::string& pathToResourceFolder) : _resourcePath(pathToResourceFolder)
  {
    LoadAssetFile("spritesheets.json", _spriteSheets);

    // remove this once the data is saved in the json...
    for (auto& item : _spriteSheets)
    {
      item.second.GenerateSheetInfo();
    }

    LoadAssetFile("animations.json", _animations);

    LoadAssetFile("actions.json", _actions);
    // get the "sheetToAnimFrame" data and store it so this doesnt have to be tied to the collection
    /*for (auto& item : _actions)
    {
      _actionEventLookupers[item.first] = AnimationEventHelper::ParseAnimationEventList(item.second.eventData, item.second.frameData, _animations[item.first].frames);
    }*/
  }

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

  std::unordered_map<std::string, SpriteSheet> _spriteSheets;
  std::unordered_map<std::string, AnimationAsset> _animations;
  std::unordered_map<std::string, ActionAsset> _actions;

  //!......
  //std::unordered_map<std::string, EventBuilderDictionary> _actionEventLookupers;
};
