#pragma once
#include "Geometry.h"

const float avg_animation_fps = 24.0f;
const float third_strike_fps = 16.0f;

const float animation_fps = third_strike_fps;

const int hitstopLight = 10;
const int hitstopMedium = 10;
const int hitstopHeavy = 10;

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

enum class AnchorPoint
{
  TL, TR, BL, BR, Size
};

struct AnimationInfo
{
  std::string sheet;
  int rows, columns;
  int startIndexOnSheet;
  int frames;
  AnchorPoint anchor;
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
