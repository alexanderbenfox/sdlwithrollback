#include "AssetManagement/StaticAssets/CharacterConfig.h"
#include "AssetManagement/StaticAssets/StaticAssetUtils.h"

float RyuConfig::moveSpeed;
float RyuConfig::jumpHeight;

std::unordered_map<std::string, AnimationInfo> RyuConfig::normalAnimations = 
{
  { "Idle", {"spritesheets\\idle_and_walking.png", 6, 6, 0, 10, AnchorPoint::TL}},
  { "WalkF", {"spritesheets\\idle_and_walking.png", 6, 6, 10, 12, AnchorPoint::TL}},
  { "WalkB", {"spritesheets\\idle_and_walking.png", 6, 6, 22, 11, AnchorPoint::TR}},
  { "Jumping", {"spritesheets\\idle_walking_jab_jump_crouch.png", 8, 10, 41, 19, AnchorPoint::BL}},
  { "Falling", {"spritesheets\\idle_walking_jab_jump_crouch.png", 8, 10, 60, 13, AnchorPoint::BL}},
  { "Crouching", {"spritesheets\\crouching.png", 4, 5, 0, 4, AnchorPoint::BL}},
  { "Crouch", {"spritesheets\\crouching.png", 4, 5, 12, 5, AnchorPoint::BL}},
  { "JumpingLight", {"spritesheets\\jlp.png", 4, 4, 0, 14, AnchorPoint::TL}},
  { "JumpingMedium", {"spritesheets\\jlp.png", 4, 4, 0, 14, AnchorPoint::TL}},
  { "JumpingHeavy", {"spritesheets\\jlp.png", 4, 4, 0, 14, AnchorPoint::TL}},
  { "Block", {"spritesheets\\block_mid_hitstun.png", 8, 7, 0, 4, AnchorPoint::BL}},
  {"LightHitstun", {"spritesheets\\block_mid_hitstun.png", 8, 7, 4, 3, AnchorPoint::BL}},
  {"LightHitstun2", {"spritesheets\\block_mid_hitstun.png", 8, 7, 37, 4, AnchorPoint::BL}},
  {"MediumHitstun", {"spritesheets\\block_mid_hitstun.png", 8, 7, 8, 10, AnchorPoint::BL}},
  {"MediumHitstun2", {"spritesheets\\block_mid_hitstun.png", 8, 7, 19, 9, AnchorPoint::BL}},
  {"HeavyHitstun", {"spritesheets\\block_mid_hitstun.png", 8, 7, 42, 12, AnchorPoint::BL}},
  {"LaunchHitstun", {"spritesheets\\block_mid_hitstun.png", 8, 7, 27, 9, AnchorPoint::BL}}
};

std::unordered_map<std::string, std::tuple<AnimationInfo, FrameData, std::string, AnimationDebuggingInfo>> RyuConfig::attackAnimations = 
{
  { "CrouchingLight",
    std::make_tuple(
      AnimationInfo{"spritesheets\\grounded_attacks.png", 8, 10, 9, 7, AnchorPoint::BL},
      FrameData{4, 3, 7, 3, 3, 1, Vector2<float>(120.0f, -100.0f), hitstopLight},
      "spritesheets\\grounded_attacks_hitboxes.png", AnimationDebuggingInfo{0})
  },
  { "CrouchingMedium",
    std::make_tuple(
      AnimationInfo{"spritesheets\\grounded_attacks.png", 8, 10, 16, 11, AnchorPoint::BL},
      FrameData{10, 3, 13, 5, 0, 1, Vector2<float>(120.0f, -600.0f), hitstopMedium},
      "spritesheets\\grounded_attacks_hitboxes.png", AnimationDebuggingInfo{0})
  },
  { "CrouchingHeavy",
    std::make_tuple(
      AnimationInfo{"spritesheets\\grounded_attacks.png", 8, 10, 27, 11, AnchorPoint::BL},
      FrameData{6, 4, 24, 5, 2, 1, Vector2<float>(120.0f, -900.0f), hitstopHeavy},
      "spritesheets\\grounded_attacks_hitboxes.png", AnimationDebuggingInfo{0})
  },
  { "StandingLight",
    std::make_tuple(
      AnimationInfo{"spritesheets\\grounded_attacks.png", 8, 10, 38, 7, AnchorPoint::BL},
      FrameData{4, 2, 7, 3, -2, 1, Vector2<float>(120.0f, -100.0f), hitstopLight},
      "spritesheets\\grounded_attacks_hitboxes.png", AnimationDebuggingInfo{0})
  },
  { "StandingMedium",
    std::make_tuple(
      AnimationInfo{"spritesheets\\grounded_attacks.png", 8, 10, 45, 9, AnchorPoint::BL},
      FrameData{7, 3, 12, 4, 2, 1, Vector2<float>(120.0f, -100.0f), hitstopMedium},
      "spritesheets\\grounded_attacks_hitboxes.png", AnimationDebuggingInfo{0})
  },
  { "StandingHeavy", 
    std::make_tuple(
      AnimationInfo{"spritesheets\\grounded_attacks.png", 8, 10, 53, 12, AnchorPoint::BL},
      FrameData{ 8, 3, 20, 7, -6, 1, Vector2<float>(120.0f, -400.0f), hitstopHeavy},
      "spritesheets\\grounded_attacks_hitboxes.png", AnimationDebuggingInfo{0})
  },
  { "SpecialMove1",
    std::make_tuple(
      AnimationInfo{"spritesheets\\grounded_attacks.png", 8, 10, 65, 14, AnchorPoint::BL},
      FrameData{ 11, 3, 28, -3, -6, 1, Vector2<float>(400.0f, 100.0f), hitstopHeavy },
      "spritesheets\\grounded_attacks_hitboxes.png", AnimationDebuggingInfo{0})
  }
};


AnimationCollection& RyuConfig::Animations()
{
  if (!_animLoaded)
  {
    StaticAssetUtils::LoadAnimations(normalAnimations, attackAnimations, _animations);
    _animLoaded = true;
  }
  return _animations;
}

bool RyuConfig::_animLoaded = false;

AnimationCollection RyuConfig::_animations;