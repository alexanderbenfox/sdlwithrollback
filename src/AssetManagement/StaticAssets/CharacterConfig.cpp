#include "AssetManagement/StaticAssets/CharacterConfig.h"
#include "AssetManagement/StaticAssets/StaticAssetUtils.h"

float RyuConfig::moveSpeed;
float RyuConfig::jumpHeight;

std::unordered_map<std::string, AnimationInfo> RyuConfig::normalAnimations = 
{
  { "Idle", {idleAndWalking, 0, 10, AnchorPoint::TL}},
  { "WalkF", {idleAndWalking, 10, 12, AnchorPoint::TL}},
  { "WalkB", {idleAndWalking, 22, 11, AnchorPoint::TR}},
  { "Jumping", {jumpFall, 41, 19, AnchorPoint::BL}},
  { "Falling", {jumpFall, 60, 13, AnchorPoint::BL}},
  { "Crouching", {crouching, 0, 4, AnchorPoint::BL}},
  { "Crouch", {crouching, 12, 5, AnchorPoint::BL}},
  { "Block", {blockAndHitstun, 0, 4, AnchorPoint::BL}},
  {"LightHitstun", {blockAndHitstun, 4, 3, AnchorPoint::BL}},
  {"LightHitstun2", {blockAndHitstun, 37, 4, AnchorPoint::BL}},
  {"MediumHitstun", {blockAndHitstun, 8, 10, AnchorPoint::BL}},
  {"MediumHitstun2", {blockAndHitstun, 19, 9, AnchorPoint::BL}},
  {"HeavyHitstun", {blockAndHitstun, 42, 12, AnchorPoint::BL}},
  {"LaunchHitstun", {blockAndHitstun, 27, 9, AnchorPoint::BL}},
  {"BackDash", {dashes, 0, 6, AnchorPoint::BL}},
  {"ForwardDash", {dashes, 6, 6, AnchorPoint::BL}},
  {"Win", {winLoseSpriteSheet, 0, 6, AnchorPoint::BL}},
  {"KO", {winLoseSpriteSheet, 6, 17, AnchorPoint::BL}}
};

std::unordered_map<std::string, std::tuple<AnimationInfo, FrameData, std::string, AnimationDebuggingInfo>> RyuConfig::attackAnimations = 
{
  { "CrouchingLight",
    std::make_tuple(
      AnimationInfo{groundedAttacks, 9, 7, AnchorPoint::BL},
      FrameData{4, 3, 7, 3, 3, 10, Vector2<float>(120.0f, -100.0f), hitstopLight},
      "spritesheets\\grounded_attacks_hitboxes.png", AnimationDebuggingInfo{0})
  },
  { "CrouchingMedium",
    std::make_tuple(
      AnimationInfo{groundedAttacks, 16, 11, AnchorPoint::BL},
      FrameData{10, 3, 13, 5, 0, 10, Vector2<float>(120.0f, -600.0f), hitstopMedium},
      "spritesheets\\grounded_attacks_hitboxes.png", AnimationDebuggingInfo{0})
  },
  { "CrouchingHeavy",
    std::make_tuple(
      AnimationInfo{groundedAttacks, 27, 11, AnchorPoint::BL},
      FrameData{6, 4, 24, 5, 2, 10, Vector2<float>(120.0f, -900.0f), hitstopHeavy},
      "spritesheets\\grounded_attacks_hitboxes.png", AnimationDebuggingInfo{0})
  },
  { "StandingLight",
    std::make_tuple(
      AnimationInfo{groundedAttacks, 38, 7, AnchorPoint::BL},
      FrameData{4, 3, 7, 3, -2, 10, Vector2<float>(120.0f, -100.0f), hitstopLight},
      "spritesheets\\grounded_attacks_hitboxes.png", AnimationDebuggingInfo{0})
  },
  { "StandingMedium",
    std::make_tuple(
      AnimationInfo{groundedAttacks, 45, 9, AnchorPoint::BL},
      FrameData{7, 3, 12, 4, 2, 10, Vector2<float>(120.0f, -100.0f), hitstopMedium},
      "spritesheets\\grounded_attacks_hitboxes.png", AnimationDebuggingInfo{0})
  },
  { "StandingHeavy", 
    std::make_tuple(
      AnimationInfo{groundedAttacks, 53, 12, AnchorPoint::BL},
      FrameData{ 8, 3, 20, 7, -6, 10, Vector2<float>(120.0f, -400.0f), hitstopHeavy},
      "spritesheets\\grounded_attacks_hitboxes.png", AnimationDebuggingInfo{0})
  },
  { "JumpingLight",
    std::make_tuple(
      AnimationInfo{jumpingAttacks, 0, 8, AnchorPoint::TL},
      // here we do -50 + 7 so that you are +7 if you land at the exact moment you hit
      FrameData{ 4, 2, 50, -50 + 7, -50 + 7, 10, Vector2<float>(120.0f, -200.0f), hitstopHeavy},
      "spritesheets\\jumping_attacks_hitboxes.png",  AnimationDebuggingInfo{0})
  },
  { "JumpingMedium",
    std::make_tuple(
      AnimationInfo{jumpingAttacks, 9, 9, AnchorPoint::TL},
      FrameData{ 5, 6, 50, -50 + 9, -50 + 9, 10, Vector2<float>(120.0f, 200.0f), hitstopHeavy},
      "spritesheets\\jumping_attacks_hitboxes.png",  AnimationDebuggingInfo{0})
  },
  { "JumpingHeavy",
    std::make_tuple(
      AnimationInfo{jumpingAttacks, 17, 8, AnchorPoint::TL},
      // here we do -50 + 11 so that you are +11 if you land at the exact moment you hit
      FrameData{ 6, 3, 50, -50 + 11, -50 + 11, 10, Vector2<float>(200.0f, 300.0f), hitstopHeavy},
      "spritesheets\\jumping_attacks_hitboxes.png",  AnimationDebuggingInfo{0})
  },
  { "SpecialMove1",
    std::make_tuple(
      AnimationInfo{groundedAttacks, 65, 14, AnchorPoint::BL},
      FrameData{ 11, 3, 28, -3, -6, 10, Vector2<float>(400.0f, 100.0f), hitstopHeavy },
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
