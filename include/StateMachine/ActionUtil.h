#pragma once
#include "StateMachine/IAction.h"

//______________________________________________________________________________
struct ActionParams
{
  static int nDashFrames;
  static float baseWalkSpeed;
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

//______________________________________________________________________________
// partial specialization
template <StanceState Stance> IAction* GetAttacksFromNeutral(const InputBuffer& rawInput, bool facingRight);

//______________________________________________________________________________
IAction* CheckHits(const InputState& rawInput, const StateComponent& context);

//______________________________________________________________________________
IAction* StateLockedHandleInput(const InputState& rawInput, const StateComponent& context, IAction* action, bool actionComplete);
