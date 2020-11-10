#include "Managers/AnimationCollectionManager.h"
#include "Core/Utility/FilePath.h"
#include "Core/Utility/JsonFile.h"

unsigned int AnimationCollectionManager::GetCollectionID(std::string_view name)
{
  return _idLookupTable[name.data()];
}

AnimationCollection& AnimationCollectionManager::GetCollection(unsigned int ID)
{
  return _collections[ID];
}

Animation* AnimationCollectionManager::GetAnimationData(unsigned int collectionID, std::string_view animationName)
{
  return GetCollection(collectionID).GetAnimation(animationName.data());
}

void AnimationCollectionManager::AddNewCharacter(const std::string& characterName)
{
  FilePath path = _characterDir;
  path.Append(characterName.c_str());
  path.Create();
  _characters.emplace(characterName, path.GetPath());
}

AnimationCollectionManager::AnimationCollectionManager() : _livingCollectionCount(0)
{
  FilePath jsonDir(ResourceManager::Get().GetResourcePath() + "json");

  // Load general animations like sfx
  JsonFile gSpritesFile(StringUtils::CorrectPath(jsonDir.GetPath() + "/general/spritesheets.json"));
  AssetLibrary<SpriteSheet>::LoadJsonData(gSpritesFile);
  
  JsonFile gAnimsFile(StringUtils::CorrectPath(jsonDir.GetPath() + "/general/animations.json"));
  std::unordered_map<std::string, AnimationAsset> generalAnimations;
  gAnimsFile.LoadContentsIntoMap(generalAnimations);

  unsigned int generalID = RegisterNewCollection("General");
  for (const auto& animation : generalAnimations)
  {
    _collections[generalID].RegisterAnimation(animation.first, animation.second);
  }

  //! load more complex character collections
  _characterDir = jsonDir;
  _characterDir.Append("characters");

  // get all characters from folder
  auto characters = _characterDir.GetSubDirectories();
  for (const auto& path : characters)
  {
    std::string characterName = path.GetLast();
    _characters.emplace(characterName, path.GetPath());
    unsigned int id = RegisterCharacterCollection(characterName, _characters.at(characterName));
  }
}

unsigned int AnimationCollectionManager::RegisterCharacterCollection(const std::string& lookUpString, const CharacterConfiguration& configFiles)
{
  assert(_livingCollectionCount < 10 && "Cannot load more animation collections. Max count exceeded");

  unsigned int assignedID = RegisterNewCollection(lookUpString);

  AnimationCollection& newCollection = _collections[assignedID];

  // clear collection before assigning animations
  newCollection.Clear();

  // register all animations and actions as events
  for (const auto& animation : configFiles.GetAnimationConfig())
  {
    newCollection.RegisterAnimation(animation.first, animation.second);
  }

  for (const auto& action : configFiles.GetActionConfig())
  {
    newCollection.SetAnimationEvents(action.first, action.second.eventData, action.second.frameData);
  }
  return assignedID;
}

unsigned int AnimationCollectionManager::RegisterNewCollection(const std::string& lookUpString)
{
  if (_idLookupTable.find(lookUpString) != _idLookupTable.end())
    return _idLookupTable[lookUpString];

  assert(_livingCollectionCount < 10 && "Cannot load more animation collections. Max count exceeded");

  unsigned int assignedID = _livingCollectionCount++;
  _idLookupTable[lookUpString] = assignedID;

  return assignedID;
}
