#pragma once
#include <unordered_map>
#include "ComponentConst.h"
#include "AssetManagement/Animation.h"

struct AnimationDebuggingInfo
{
  int frame;
};

class ICharacterConfig {};

class RyuConfig : public ICharacterConfig
{
public:
  //!
  static AnimationCollection& Animations();
  //!
  static float moveSpeed;
  //!
  static float jumpHeight;

protected:

  //!
  static std::unordered_map<std::string, AnimationInfo> normalAnimations;
  //! contains animation info, frame data, and hitbox sheet
  static std::unordered_map<std::string, std::tuple<AnimationInfo, FrameData, std::string, AnimationDebuggingInfo>> attackAnimations;

  static bool _animLoaded;
  static AnimationCollection _animations;

};