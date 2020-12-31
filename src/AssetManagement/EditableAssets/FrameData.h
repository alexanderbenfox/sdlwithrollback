#pragma once
#include "AssetManagement/EditableAssets/IJsonLoadable.h"
#include "Core/Math/Vector2.h"
#include "Core/FightingGameTypes/HitType.h"

struct FrameData : public IJsonLoadable
{
  FrameData() = default;
  FrameData(int su, int act, int rec, int oha, int oba, int dmg, Vector2<float> k, int hs) : startUp(su), active(act), recover(rec), onHitAdvantage(oha), onBlockAdvantage(oba), damage(dmg), knockback(k), hitstop(hs) {}
  // # of start up frames, active frames, and recovery frames
  int startUp, active, recover;
  // # of frames the receiver should be stunned on hit or block after attacker returns to neutral (can be + or -)
  int onHitAdvantage, onBlockAdvantage;
  // damage that the move does
  int damage;
  // knockback vector oriented from attack's source
  Vector2<float> knockback;
  // number of frames of action pause
  int hitstop;
  //
  bool isThrow = false;
  //
  bool knockdown = false;
  //
  HitType type = HitType::Mid;

  virtual void Load(const Json::Value& json) override;

  virtual void Write(Json::Value& json) const override;

  virtual void DisplayInEditor() override;
};
