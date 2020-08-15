// define all of the global static vars in this file
#include "Globals.h"

//______________________________________________________________________________
int GlobalVars::HitStopFramesOnHit = 10;
int GlobalVars::HitStopFramesOnBlock = 10;
int GlobalVars::nDashFrames = 23;
float GlobalVars::BaseWalkSpeed = 520.0f;

bool GlobalVars::ShowHitEffects = true;

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
