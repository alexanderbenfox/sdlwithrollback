#pragma once
#include "AssetManagement/StaticAssets/AnimationAssetData.h"
#include <unordered_map>

const SpriteSheet idleAndWalking("spritesheets\\idle_and_walking.png", 6, 6, false);
const SpriteSheet jumpFall("spritesheets\\idle_walking_jab_jump_crouch.png", 8, 10, false);
const SpriteSheet crouching("spritesheets\\crouching.png", 4, 5, false);
const SpriteSheet groundedAttacks("spritesheets\\grounded_attacks.png", 8, 10, false);
const SpriteSheet jumpingAttacks("spritesheets\\jumping_attacks.png", 5, 5, false);
const SpriteSheet hitstun("spritesheets\\hitstun_standing&crouching.png", 9, 7, false);
const SpriteSheet blocks("spritesheets\\blocks.png", 4, 4, false);
const SpriteSheet throws("spritesheets\\throws.png", 8, 5, false);
const SpriteSheet winLoseSpriteSheet("spritesheets\\winlose.png", 6, 4, false);
const SpriteSheet dashes("spritesheets\\dashes.png", 4, 3, false);
const SpriteSheet specialMoves("spritesheets\\special_moves.png", 10, 8, false);
const SpriteSheet knockdowns("spritesheets\\knockdowns.png", 9, 6, false);

const SpriteSheet fireballSheet("spritesheets\\coolfireballspritesheet.png", 1, 4, false);
const SpriteSheet hitblockSparks("sfx\\hitblocksparks.png", 8, 7, false);

// Careful with static initialization here....
struct RyuAnimationData
{
  //!
  static std::unordered_map<std::string, AnimationInfo> normalAnimations;
  //! contains animation info, frame data, and hitbox sheet
  static std::unordered_map<std::string, AttackAnimationData> attackAnimations;
};
