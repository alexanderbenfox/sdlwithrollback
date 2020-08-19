#include "AssetManagement/AnimationCollectionManager.h"

#include "AssetManagement/StaticAssets/StaticAssetUtils.h"
#include "AssetManagement/StaticAssets/AnimationAssets.h"

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

AnimationCollectionManager::AnimationCollectionManager() : _livingCollectionCount(0)
{
  //! load all of the generic animations first
  AnimationInfo fireballNormal(fireballSheet.src, fireballSheet, 0, 4, AnchorPoint::BL);
  LoadSingleAnimation("General", "Fireball", fireballNormal);

  AnimationInfo sfxHitSparks(hitblockSparks.src, hitblockSparks, 0, 28, AnchorPoint::BL);
  AnimationInfo sfxBlockSparks(hitblockSparks.src, hitblockSparks, 28, 28, AnchorPoint::BL);
  LoadSingleAnimation("Sparks", "HitSparks", sfxHitSparks);
  LoadSingleAnimation("Sparks", "BlockSparks", sfxBlockSparks);

  //! load more complex character collections
  LoadCharacterCollection("Ryu", RyuAnimationData::normalAnimations, RyuAnimationData::attackAnimations);

}

void AnimationCollectionManager::LoadCharacterCollection(const std::string& lookUpString,
  std::unordered_map<std::string, AnimationInfo>& normalAnimations,
  std::unordered_map<std::string, AttackAnimationData>& attackAnimations)
{
  assert(_livingCollectionCount < 10 && "Cannot load more animation collections. Max count exceeded");

  unsigned int assignedID = RegisterNewCollection(lookUpString);

  AnimationCollection& newCollection = _collections[assignedID];
  StaticAssetUtils::LoadAnimations(normalAnimations, attackAnimations, newCollection);
  StaticAssetUtils::CreateAnimationDebug(normalAnimations, attackAnimations, newCollection);
}

void AnimationCollectionManager::LoadSingleAnimation(std::string_view collection, std::string_view name, const AnimationInfo& animInfo)
{
  unsigned int ID = RegisterNewCollection(collection.data());
  StaticAssetUtils::LoadNormal(name.data(), animInfo, _collections[ID]);
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
