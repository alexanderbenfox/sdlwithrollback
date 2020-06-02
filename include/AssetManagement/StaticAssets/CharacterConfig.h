#pragma once
#include <unordered_map>
#include "ComponentConst.h"

struct AnimationDebuggingInfo
{
  int frame;
};

class RyuConfig
{
public:
  static float moveSpeed;
  static float jumpHeight;
  static std::unordered_map<std::string, AnimationInfo> normalAnimations;
  // contains animation info, frame data, and hitbox sheet
  static std::unordered_map<std::string, std::tuple<AnimationInfo, FrameData, std::string, AnimationDebuggingInfo>> attackAnimations;
};