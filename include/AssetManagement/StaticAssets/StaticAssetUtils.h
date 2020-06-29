#pragma once
#include "CharacterConfig.h"
#include "AnimationAssetData.h"

class StaticAssetUtils
{
public:
  static void LoadAnimations(std::unordered_map<std::string, AnimationInfo>& normalAnimations,
    std::unordered_map<std::string, std::tuple<AnimationInfo, FrameData, std::string, AnimationDebuggingInfo>>& attackAnimations,
    AnimationCollection& collection);

  static std::string jsonWriteOutLocation;

  static bool loadFromFile;
};
