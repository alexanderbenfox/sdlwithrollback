#pragma once
#include <unordered_map>
#include "ComponentConst.h"
#include "AssetManagement/Animation.h"

struct AnimationDebuggingInfo
{
  int frame;
};

const SpriteSheet idleAndWalking("spritesheets\\idle_and_walking.png", 6, 6, false);
const SpriteSheet jumpFall("spritesheets\\idle_walking_jab_jump_crouch.png", 8, 10, false);
const SpriteSheet crouching("spritesheets\\crouching.png", 4, 5, false);
const SpriteSheet groundedAttacks("spritesheets\\grounded_attacks.png", 8, 10, false);
const SpriteSheet jumpingAttacks("spritesheets\\jumping_attacks.png", 5, 5, false);
const SpriteSheet blockAndHitstun("spritesheets\\block_mid_hitstun.png", 8, 7, false);
const SpriteSheet blocks("spritesheets\\blocks.png", 4, 4, false);
const SpriteSheet throws("spritesheets\\throws.png", 8, 5, false);
const SpriteSheet winLoseSpriteSheet("spritesheets\\winlose.png", 4, 4, false);
const SpriteSheet dashes("spriteSheets\\dashes.png", 4, 3, false);

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