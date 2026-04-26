#include "Managers/AnimationCollectionManager.h"
#include "AssetManagement/EditableAssets/AssetLibraryImpl.h"
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

IAnimation* AnimationCollectionManager::GetAnimationData(unsigned int collectionID, std::string_view animationName)
{
  return GetCollection(collectionID).GetAnimation(animationName.data());
}

void AnimationCollectionManager::AddNewCharacter(const std::string& characterName)
{
  FilePath path = _characterDir;
  path.Append(characterName.c_str());
  path.Create();
  _characterNames.push_back(characterName);
  LoadCharacterFromJson(characterName, path.GetPath());
}

void AnimationCollectionManager::ReloadCharacter(const std::string& name)
{
  auto it = _idLookupTable.find(name);
  if (it == _idLookupTable.end())
    return;

  AnimationCollection& c = _collections[it->second];
  c.Clear();

  FilePath charDir = _characterDir;
  charDir.Append(name.c_str());

  // Re-load animations and actions from JSON
  std::unordered_map<std::string, AnimationAsset> animations;
  std::unordered_map<std::string, ActionAsset> actions;

  JsonFile animFile(StringUtils::CorrectPath(charDir.GetPath() + "/animations.json"));
  if (animFile.IsValid())
    animFile.LoadContentsIntoMap(animations);

  JsonFile actFile(StringUtils::CorrectPath(charDir.GetPath() + "/actions.json"));
  if (actFile.IsValid())
    actFile.LoadContentsIntoMap(actions);

  for (const auto& animation : animations)
    c.RegisterAnimation(animation.first, animation.second);

  for (const auto& action : actions)
    c.SetAnimationEvents(action.first, action.second.eventData, action.second.frameData);
}

AnimationCollectionManager::AnimationCollectionManager()
{
  FilePath jsonDir(ResourceManager::Get().GetResourcePath() + "json");

  // Load general animations like sfx
  JsonFile gSpritesFile(StringUtils::CorrectPath(jsonDir.GetPath() + "/general/spritesheets.json"));
  ResourceManager::Get().gSpriteSheets.LoadJsonData(gSpritesFile);

  JsonFile gAnimsFile(StringUtils::CorrectPath(jsonDir.GetPath() + "/general/animations.json"));
  gAnimsFile.LoadContentsIntoMap(_generalAnimations);

  unsigned int generalID = RegisterNewCollection("General");
  for (const auto& animation : _generalAnimations)
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
    _characterNames.push_back(characterName);
    LoadCharacterFromJson(characterName, path.GetPath());
  }
}

void AnimationCollectionManager::LoadCharacterFromJson(const std::string& name, const std::string& charDir)
{
  // Load spritesheets into the global ResourceManager
  JsonFile spritesFile(StringUtils::CorrectPath(charDir + "/spritesheets.json"));
  if (spritesFile.IsValid())
    ResourceManager::Get().gSpriteSheets.LoadJsonData(spritesFile);

  // Load animations and actions into local maps (discarded after registration)
  std::unordered_map<std::string, AnimationAsset> animations;
  std::unordered_map<std::string, ActionAsset> actions;

  JsonFile animFile(StringUtils::CorrectPath(charDir + "/animations.json"));
  if (animFile.IsValid())
    animFile.LoadContentsIntoMap(animations);

  JsonFile actFile(StringUtils::CorrectPath(charDir + "/actions.json"));
  if (actFile.IsValid())
    actFile.LoadContentsIntoMap(actions);

  unsigned int assignedID = RegisterNewCollection(name);
  AnimationCollection& collection = _collections[assignedID];
  collection.Clear();

  for (const auto& animation : animations)
    collection.RegisterAnimation(animation.first, animation.second);

  for (const auto& action : actions)
    collection.SetAnimationEvents(action.first, action.second.eventData, action.second.frameData);
}

unsigned int AnimationCollectionManager::RegisterNewCollection(const std::string& lookUpString)
{
  if (_idLookupTable.find(lookUpString) != _idLookupTable.end())
    return _idLookupTable[lookUpString];

  unsigned int assignedID = static_cast<unsigned int>(_collections.size());
  _collections.emplace_back();
  _idLookupTable[lookUpString] = assignedID;

  return assignedID;
}

void AnimationCollectionManager::EditGeneralAnimations()
{
  static int gAnimCounter = 0;
  for (auto& item : _generalAnimations)
  {
    const int fieldHeight = 25;
    std::string name = "##item:" + std::to_string(gAnimCounter++);
    ImGui::BeginChild(name.c_str(), ImVec2(500, 6 * fieldHeight), true);
    ImGui::Text("%s", item.first.c_str());
    item.second.DisplayInEditor();
    if (ImGui::Button("Set Anchor In-Game"))
    {
      GAnimArchive.GetCollection(GetCollectionID("General")).GetSpriteAnimation(item.first)->SetAnchorPoint(item.second.anchor, item.second.GetAnchorPosition(0));
    }
    ImGui::EndChild();
  }

  if (ImGui::Button("Save General Animations"))
  {
    FilePath jsonDir(ResourceManager::Get().GetResourcePath() + "json");
    JsonFile gAnimsFile(StringUtils::CorrectPath(jsonDir.GetPath() + "/general/animations.json"));
    gAnimsFile.SaveContentsIntoFile(_generalAnimations);
  }

  gAnimCounter = 0;
}
