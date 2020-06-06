#pragma once
#include <unordered_map>
#include "ComponentConst.h"

struct AnimationDebuggingInfo
{
  int frame;
};

const SpriteSheet idleAndWalking("spritesheets\\idle_and_walking.png", 6, 6, false);
const SpriteSheet jumpFall("spritesheets\\idle_walking_jab_jump_crouch.png", 8, 10, false);
const SpriteSheet crouching("spritesheets\\crouching.png", 4, 5, false);
const SpriteSheet groundedAttacks("spritesheets\\grounded_attacks.png", 8, 10, false);
const SpriteSheet jumpingAttacks("spritesheets\\jlp.png", 4, 4, false);
const SpriteSheet blockAndHitstun("spritesheets\\block_mid_hitstun.png", 8, 7, false);

class RyuConfig
{
public:
  static float moveSpeed;
  static float jumpHeight;
  static std::unordered_map<std::string, AnimationInfo> normalAnimations;
  // contains animation info, frame data, and hitbox sheet
  static std::unordered_map<std::string, std::tuple<AnimationInfo, FrameData, std::string, AnimationDebuggingInfo>> attackAnimations;
};