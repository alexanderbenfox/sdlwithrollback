#pragma once
#include "Geometry.h"

const float avg_animation_fps = 24.0f;
const float third_strike_fps = 16.0f;

const float animation_fps = third_strike_fps;

struct FrameData
{
  // Member initialization
  FrameData() = default;
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
};

class ConstComponentIDGenerator
{
public:
  static int ID;
  static int NextID()
  {
      return ID++;
  }
};
