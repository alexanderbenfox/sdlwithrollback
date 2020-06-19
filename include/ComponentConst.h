#pragma once
#include "Core/Math/Vector2.h"

const float avg_animation_fps = 24.0f;
const float third_strike_fps = 16.0f;

const float animation_fps = third_strike_fps;

const int hitstopLight = 10;
const int hitstopMedium = 10;
const int hitstopHeavy = 10;

//! Set our preferred type (SDL or GL) to be rendered by the system
class GLTexture;
typedef GLTexture RenderType;

struct FrameData
{
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

struct SpriteSheet
{
  SpriteSheet(const char* src, int rows, int columns);
  SpriteSheet(const char* src, int rows, int columns, bool dontLoad) : src(src), rows(rows), columns(columns) {}
  //!
  std::string src;
  //!
  Vector2<int> frameSize;
  Vector2<int> sheetSize;
  //!
  int rows, columns;
};

struct AnimationInfo
{
  SpriteSheet sheet;
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
