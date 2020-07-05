#pragma once
#include "CharacterConfig.h"
#include "AnimationAssetData.h"

class StaticAssetUtils
{
public:
  static void LoadAnimations(std::unordered_map<std::string, AnimationInfo>& normalAnimations, std::unordered_map<std::string, AttackAnimationData>& attackAnimations,
    AnimationCollection& collection);

  static void CreateAnimationDebug(std::unordered_map<std::string, AnimationInfo>& normalAnimations, std::unordered_map<std::string, AttackAnimationData>& attackAnimations,
    AnimationCollection& collection);

  static void LoadCollectionFromJson(AnimationCollection& collection, std::unordered_map<std::string, AttackAnimationData>& attackAnimations, const std::string& spriteSheetJsonLocation, const std::string& movesetJsonLocation);

  static std::vector<AnimationActionEventData> CreateEventDataFromHitboxSheet(const char* hitboxesSheet, const AttackAnimationData& frameData);

  static std::vector<Rect<double>> GetHitboxesFromFile(const char* hitboxesSheet, const AnimationInfo& animInfo);

  static std::string jsonWriteOutLocation;

  static bool loadFromFile;
};
