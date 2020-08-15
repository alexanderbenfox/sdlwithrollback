#pragma once
#include "AssetManagement/Animation.h"
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

class ICharacterConfig {};

class RyuConfig : public ICharacterConfig
{
public:
  //!
  static AnimationCollection& FireballAnimations();
  static AnimationCollection& Animations();
  static void Reload();
  //!
  static float moveSpeed;
  //!
  static float jumpHeight;

protected:

  //!
  static std::unordered_map<std::string, AnimationInfo> normalAnimations;
  //! contains animation info, frame data, and hitbox sheet
  static std::unordered_map<std::string, AttackAnimationData> attackAnimations;

  static bool _animLoaded, _fireballsLoaded;
  static AnimationCollection _animations;

  static AnimationCollection _fireballAnimations;

};

struct AnimCollectionsGetter
{
  static AnimationCollection& GetCollection(const std::string& name)
  {
    if (name == "Ryu")
      return RyuConfig::Animations();
    if (name == "Fireball")
      return RyuConfig::FireballAnimations();

    return RyuConfig::Animations();
  }
};