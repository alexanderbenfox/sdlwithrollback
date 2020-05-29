#include "GameManagement.h"
#include "ResourceManager.h"
#include "AssetManagement/StaticAssets/CharacterConfig.h"

AnimationCollection& AnimationResources::RyuAnimations()
{
  if(!_ryuAnimLoaded)
  {
    _ryuAnimations = LoadRyuAnimations();
    _ryuAnimLoaded = true;
  }
  return _ryuAnimations;
}

bool AnimationResources::_ryuAnimLoaded = false;

AnimationCollection AnimationResources::_ryuAnimations;

AnimationCollection AnimationResources::LoadRyuAnimations()
{
  AnimationCollection ryuAnimations;

  //hack cause i suck
  auto it = RyuConfig::normalAnimations.find("Idle");
  ryuAnimations.RegisterAnimation("Idle", it->second.sheet.c_str(), it->second.rows, it->second.columns, it->second.startIndexOnSheet, it->second.frames, it->second.anchor);

  for(auto& anim : RyuConfig::normalAnimations)
  {
    AnimationInfo& params = anim.second;
    ryuAnimations.RegisterAnimation(anim.first, params.sheet.c_str(), params.rows, params.columns, params.startIndexOnSheet, params.frames, params.anchor);
  }
  for(auto& anim : RyuConfig::attackAnimations)
  {
    AnimationInfo& params = std::get<AnimationInfo>(anim.second);
    ryuAnimations.RegisterAnimation(anim.first, params.sheet.c_str(), params.rows, params.columns, params.startIndexOnSheet, params.frames, params.anchor);
    ryuAnimations.SetHitboxEvents(anim.first, std::get<2>(anim.second).c_str(), std::get<1>(anim.second)); 
  }
  return ryuAnimations;
}