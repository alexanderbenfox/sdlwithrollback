#pragma once
#include "CharacterConfig.h"

class StaticAssetUtils
{
public:
  static void LoadAnimations(std::unordered_map<std::string, AnimationInfo>& normalAnimations,
    std::unordered_map<std::string, std::tuple<AnimationInfo, FrameData, std::string, AnimationDebuggingInfo>>& attackAnimations,
    AnimationCollection& collection);
};
