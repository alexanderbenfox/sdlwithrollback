#pragma once
#include "Core/Math/Vector2.h"

const unsigned int MAX_ENTITIES = 500;
const unsigned int MAX_COMPONENTS = 128;
typedef unsigned int EntityID;

//! Number of seconds per frame (for animation and timers)
const float secPerFrame = 1.0f / 60.0f;

//! define native screen size
const int m_nativeWidth = 720;
const int m_nativeHeight = 480;

//! define basis character size
const int m_frameWidth = 112;
const int m_frameHeight = 112;

const float m_characterWidth = 58.5f;
const float m_characterHeight = 105.5f;

struct ECSGlobalStatus
{
  //! Number of registered components
  static int NRegisteredComponents;

};

//______________________________________________________________________________
//! Editable global vars for gameplay mostly
class GlobalVars
{
public:
  //! Number of hitstop frames on hit
  static int HitStopFramesOnHit;
  //! Number of hitstop frames on block
  static int HitStopFramesOnBlock;
  //! Number of frames dashed for
  static int nDashFrames;
  //! number of pixels (multiplied by delta time) moved during a walk or jump forward or back
  static float BaseWalkSpeed;
  //! force of gravity
  static Vector2<float> Gravity;
  //! force of gravity when being juggled by a combo
  static Vector2<float> JuggleGravity;
  //! inst velocity applied upwards when jump occurs
  static float JumpVelocity;
  //! Show hit and block sparks
  static bool ShowHitEffects;
  
};

//______________________________________________________________________________
class NetGlobals
{
public:
  static unsigned short LocalUDPPort;
  //! defined based on the values found in https://github.com/pond3r/ggpo/blob/master/src/apps/vectorwar/vectorwar.h
  static int FrameDelay;
};

//______________________________________________________________________________
//! Used by timed actions to interpolate
struct Interpolation
{
  //! Plateau shaped distribution curve
  struct Plateau
  {
    // constants for plateau shaped distribution function
    static float a;
    static float modifier;
    // distribution width: < 1 == narrow distro. > 1 widen
    static float d;
    //
    static float xAxisOffset;
    //! based on equation f(x) = k * (1 / (1 + x^(2*a)) where the larger a == more of a plateau
    static float F(float x, float xMax, float yMax);

  };
};

const float avg_animation_fps = 24.0f;
const float third_strike_fps = 16.0f;

const float animation_fps = third_strike_fps;

//! Set our preferred type (SDL or GL) to be rendered by the system
class GLTexture;
typedef GLTexture RenderType;


// THESE JUST NEED TO GO TBH!
//______________________________________________________________________________
//all of the possible states for animation??
enum class StanceState
{
  CROUCHING, STANDING, JUMPING, KNOCKDOWN
};

//______________________________________________________________________________
enum class ActionState
{
  NONE, BLOCKSTUN, HITSTUN, DASHING, LIGHT, MEDIUM, HEAVY
};
