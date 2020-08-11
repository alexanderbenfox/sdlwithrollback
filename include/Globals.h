#pragma once

//! Max number of components
#define MAX_COMPONENTS 128

//! Number of seconds per frame (for animation and timers)
const float secPerFrame = 1.0f / 60.0f;

//______________________________________________________________________________
//! Editable global vars
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
  //! Show hit and block sparks
  static bool ShowHitEffects;
  
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
