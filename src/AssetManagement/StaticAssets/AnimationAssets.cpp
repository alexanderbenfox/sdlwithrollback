#include "AssetManagement/StaticAssets/AnimationAssets.h"

std::unordered_map<std::string, AnimationInfo> RyuAnimationData::normalAnimations =
{
  { "Idle", AnimationInfo(idleAndWalking.src, idleAndWalking, 0, 10, AnchorPoint::TL)},
  { "WalkF", AnimationInfo(idleAndWalking.src, idleAndWalking, 10, 12, AnchorPoint::TL)},
  { "WalkB", AnimationInfo(idleAndWalking.src, idleAndWalking, 22, 11, AnchorPoint::TR)},
  { "Jumping", AnimationInfo(jumpFall.src, jumpFall, 41, 19, AnchorPoint::BL)},
  { "Falling", AnimationInfo(jumpFall.src, jumpFall, 60, 13, AnchorPoint::BL)},
  { "Crouching", AnimationInfo(crouching.src, crouching, 0, 4, AnchorPoint::BL)},
  { "Crouch", AnimationInfo(crouching.src, crouching, 12, 5, AnchorPoint::BL)},
  { "BlockLow", AnimationInfo(blocks.src, blocks, 0, 5, AnchorPoint::BL)},
  { "BlockMid", AnimationInfo(blocks.src, blocks, 5, 4, AnchorPoint::BL)},
  { "BlockHigh", AnimationInfo(blocks.src, blocks, 9, 6, AnchorPoint::BL)},
  { "LightHitstun", AnimationInfo(hitstun.src, hitstun, 0, 3, AnchorPoint::BL)},
  { "MedHitstun", AnimationInfo(hitstun.src, hitstun, 4, 11, AnchorPoint::BL)},
  { "MedHitstun2", AnimationInfo(hitstun.src, hitstun, 14, 10, AnchorPoint::BL)},
  { "HeavyHitstun", AnimationInfo(hitstun.src, hitstun, 24, 7, AnchorPoint::BL)},
  { "CrouchingHitstun", AnimationInfo(hitstun.src, hitstun, 56, 11, AnchorPoint::BL)},
  { "Knockdown_Air", AnimationInfo(knockdowns.src, knockdowns, 0, 8, AnchorPoint::BR)},
  { "Knockdown2_Air", AnimationInfo(knockdowns.src, knockdowns, 25, 16, AnchorPoint::BR)},
  { "Knockdown_HitGround", AnimationInfo(knockdowns.src, knockdowns, 8, 11, AnchorPoint::BR)},
  { "Knockdown2_HitGround", AnimationInfo(knockdowns.src, knockdowns, 41, 5, AnchorPoint::BR)},
  { "Knockdown_OnGround", AnimationInfo(knockdowns.src, knockdowns, 19, 6, AnchorPoint::BR)},
  { "BackDash", AnimationInfo(dashes.src, dashes, 0, 6, AnchorPoint::BL)},
  { "ForwardDash", AnimationInfo(dashes.src, dashes, 6, 6, AnchorPoint::BL)},
  { "ThrowMiss", AnimationInfo(throws.src, throws, 2, 4, AnchorPoint::BL)},
  { "Win", AnimationInfo(winLoseSpriteSheet.src, winLoseSpriteSheet, 0, 6, AnchorPoint::BL)},
  { "KO", AnimationInfo(winLoseSpriteSheet.src, winLoseSpriteSheet, 6, 17, AnchorPoint::BL)}
};

std::unordered_map<std::string, AttackAnimationData> RyuAnimationData::attackAnimations =
{
  { "CrouchingLight",
  AttackAnimationData(
      AnimationInfo(groundedAttacks.src, groundedAttacks, 9, 7, AnchorPoint::BL),
      FrameData(4, 3, 7, 3, 3, 10, Vector2<float>(120.0f, -100.0f), GlobalVars::HitStopFramesOnHit)) },
  { "CrouchingMedium",
   AttackAnimationData(
      AnimationInfo(groundedAttacks.src, groundedAttacks, 16, 11, AnchorPoint::BL),
      FrameData(10, 3, 13, 5, 0, 10, Vector2<float>(120.0f, -600.0f), GlobalVars::HitStopFramesOnHit)) },
  { "CrouchingHeavy",
    AttackAnimationData(
      AnimationInfo(groundedAttacks.src, groundedAttacks, 27, 11, AnchorPoint::BL),
      FrameData(6, 4, 24, 5, 2, 10, Vector2<float>(120.0f, -900.0f), GlobalVars::HitStopFramesOnHit)) },
  { "StandingLight",
    AttackAnimationData(
      AnimationInfo(groundedAttacks.src, groundedAttacks, 38, 7, AnchorPoint::BL),
      FrameData(4, 3, 7, 3, -2, 10, Vector2<float>(120.0f, -100.0f), GlobalVars::HitStopFramesOnHit)) },
{ "StandingMedium",
  AttackAnimationData(
    AnimationInfo(groundedAttacks.src, groundedAttacks, 45, 9, AnchorPoint::BL),
    FrameData(7, 3, 12, 4, 2, 10, Vector2<float>(120.0f, -100.0f), GlobalVars::HitStopFramesOnHit)) },
  { "StandingHeavy", 
    AttackAnimationData(
      AnimationInfo(groundedAttacks.src, groundedAttacks, 53, 12, AnchorPoint::BL),
      FrameData( 8, 3, 20, 7, -6, 10, Vector2<float>(120.0f, -400.0f), GlobalVars::HitStopFramesOnHit)) },
  { "JumpingLight",
    AttackAnimationData(
      AnimationInfo(jumpingAttacks.src, jumpingAttacks, 0, 8, AnchorPoint::TL),
      // here we do -50 + 7 so that you are +7 if you land at the exact moment you hit
      FrameData( 4, 2, 50, -50 + 7, -50 + 7, 10, Vector2<float>(120.0f, -200.0f), GlobalVars::HitStopFramesOnHit)) },
  { "JumpingMedium",
    AttackAnimationData(
      AnimationInfo(jumpingAttacks.src, jumpingAttacks, 9, 9, AnchorPoint::TL),
      FrameData( 5, 6, 50, -50 + 9, -50 + 9, 10, Vector2<float>(120.0f, 200.0f), GlobalVars::HitStopFramesOnHit)) },
  { "JumpingHeavy",
    AttackAnimationData(
      AnimationInfo(jumpingAttacks.src, jumpingAttacks, 17, 8, AnchorPoint::TL),
      // here we do -50 + 11 so that you are +11 if you land at the exact moment you hit
      FrameData( 6, 3, 50, -50 + 11, -50 + 11, 10, Vector2<float>(200.0f, 300.0f), GlobalVars::HitStopFramesOnHit)) },
  { "SpecialMove1",
    AttackAnimationData(
      AnimationInfo(specialMoves.src, specialMoves, 0, 14, AnchorPoint::BL),
      FrameData( 11, 3, 28, -3, -6, 10, Vector2<float>(400.0f, 100.0f), GlobalVars::HitStopFramesOnHit)) },
  { "SpecialMove2",
    AttackAnimationData(
      AnimationInfo(specialMoves.src, specialMoves, 41, 17, AnchorPoint::BL),
      FrameData(2, 5, 28, -3, -6, 10, Vector2<float>(400.0f, 100.0f), GlobalVars::HitStopFramesOnHit)) },
  { "SpecialMove3",
  AttackAnimationData(
    AnimationInfo(specialMoves.src, specialMoves, 58, 15, AnchorPoint::BL),
    FrameData(7, 3, 20, 5, -6, 10, Vector2<float>(400.0f, 100.0f), GlobalVars::HitStopFramesOnHit)) },
  { "SpecialMove4",
  AttackAnimationData(
    AnimationInfo(specialMoves.src, specialMoves, 14, 27, AnchorPoint::BL),
    FrameData(7, 18, 4, 5, -6, 10, Vector2<float>(400.0f, 100.0f), GlobalVars::HitStopFramesOnHit)) },
  { "ForwardThrow",
  AttackAnimationData(
    AnimationInfo(throws.src, throws, 6, 17, AnchorPoint::BL),
    FrameData(2, 7, 8, 20, 0, 10, Vector2<float>(400.0f, -400.0f), GlobalVars::HitStopFramesOnHit)) },
  { "BackThrow",
  AttackAnimationData(
    AnimationInfo(throws.src, throws, 23, 14, AnchorPoint::BL),
    FrameData(2, 7, 8, 20, 0, 10, Vector2<float>(-400.0f, -400.0f), GlobalVars::HitStopFramesOnHit)) },
};
