#pragma once
#include <unordered_map>
#include <vector>

#include "AssetManagement/EditableAssets/AnimationAsset.h"
#include "AssetManagement/EditableAssets/SpriteSheet.h"
#include "AssetManagement/Animation.h"
#include "Core/Utility/FilePath.h"


//! Easy access macro cause name is long :(
#define GAnimArchive AnimationCollectionManager::Get()

class AnimationCollectionManager
{
public:
  //! Static getter
  static AnimationCollectionManager& Get()
  {
    static AnimationCollectionManager manager;
    return manager;
  }
  //! Get collection ID from string name
  unsigned int GetCollectionID(std::string_view name);
  //! Get collection by ID
  AnimationCollection& GetCollection(unsigned int ID);
  //! Shortcut for getting animation data from collection
  IAnimation* GetAnimationData(unsigned int collectionID, std::string_view animationName);
  //!
  void AddNewCharacter(const std::string& characterName);

  int GetNCharacter() { return static_cast<int>(_characterNames.size()); }
  const std::vector<std::string>& GetCharacters() const { return _characterNames; }

  void EditGeneralAnimations();

  //! Reload a character's animation collection from JSON on disk
  void ReloadCharacter(const std::string& name);

private:
  //! Loads all animation collections currently
  AnimationCollectionManager();

  //! Load a character's spritesheets, animations, and actions from JSON
  void LoadCharacterFromJson(const std::string& name, const std::string& charDir);
  //! Registers new animation or gets existing if name exists in table
  unsigned int RegisterNewCollection(const std::string& lookUpString);

  //! Dynamic collection storage
  std::vector<AnimationCollection> _collections;

  //! maps name strings to index in vector
  std::unordered_map<std::string, unsigned int> _idLookupTable;

  //! Character names for gameplay queries
  std::vector<std::string> _characterNames;
  //!
  FilePath _characterDir;
  //! General animation list
  std::unordered_map<std::string, AnimationAsset> _generalAnimations;

};
