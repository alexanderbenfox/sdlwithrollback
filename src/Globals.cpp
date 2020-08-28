// define all of the global static vars in this file
#include "Globals.h"

//______________________________________________________________________________
int ECSGlobalStatus::NRegisteredComponents = 0;

//______________________________________________________________________________
int GlobalVars::HitStopFramesOnHit = 10;
int GlobalVars::HitStopFramesOnBlock = 10;
int GlobalVars::nDashFrames = 23;
float GlobalVars::BaseWalkSpeed = 520.0f;
Vector2<float> GlobalVars::Gravity = Vector2<float>(0, 2700.0f);
Vector2<float> GlobalVars::JuggleGravity = Vector2<float>(0, 2300.0f);
float GlobalVars::JumpVelocity = 1200.0f;
bool GlobalVars::ShowHitEffects = true;

//______________________________________________________________________________
unsigned short NetGlobals::LocalUDPPort = 8001;
int NetGlobals::FrameDelay = 2;

//______________________________________________________________________________
float Interpolation::Plateau::a = 2.0f;
float Interpolation::Plateau::modifier = 1.75f;
float Interpolation::Plateau::d = 0.001f;
float Interpolation::Plateau::xAxisOffset = 10.0f;


#include <cmath>

//______________________________________________________________________________
float Interpolation::Plateau::F(float x, float xMax, float yMax)
{
  x = x + xAxisOffset;
  const float pi = 3.14159265358979323846f;
  const float k = (a / pi) * std::sinf(pi / (2.0f * a));

  float scaledXValue = 0.5f * (x / xMax) - 0.5f;
  float x2a = std::powf(scaledXValue, 2.0f * a);
  return ((d * k) / (d + x2a)) * (modifier * yMax);
}
