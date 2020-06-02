#pragma once
#include "Animation.h"

class AnimationAsset
{
public:
  static AnimationCollection& RyuAnimations();

private:
  static bool _ryuAnimLoaded;
  static AnimationCollection _ryuAnimations;
  static void LoadRyuAnimations();
};
