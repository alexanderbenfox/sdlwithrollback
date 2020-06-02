#include "GameManagement.h"
#include "ResourceManager.h"
#include "AssetManagement/StaticAssets/CharacterConfig.h"
#include "DebugGUI/GUIController.h"
#include "DebugGUI/GLTexture.h"

AnimationCollection& AnimationResources::RyuAnimations()
{
  if(!_ryuAnimLoaded)
  {
    LoadRyuAnimations();
    _ryuAnimLoaded = true;
  }
  return _ryuAnimations;
}

bool AnimationResources::_ryuAnimLoaded = false;

AnimationCollection AnimationResources::_ryuAnimations;

void AnimationResources::LoadRyuAnimations()
{
  //hack cause i suck
  auto it = RyuConfig::normalAnimations.find("Idle");
  _ryuAnimations.RegisterAnimation("Idle", it->second.sheet.c_str(), it->second.rows, it->second.columns, it->second.startIndexOnSheet, it->second.frames, it->second.anchor);

  for(auto& anim : RyuConfig::normalAnimations)
  {
    AnimationInfo& params = anim.second;
    _ryuAnimations.RegisterAnimation(anim.first, params.sheet.c_str(), params.rows, params.columns, params.startIndexOnSheet, params.frames, params.anchor);
  }
  for(auto& anim : RyuConfig::attackAnimations)
  {
    std::string animName = anim.first;
    std::string hitboxSheet = std::get<2>(anim.second);
    AnimationInfo& params = std::get<AnimationInfo>(anim.second);
    FrameData& frameData = std::get<1>(anim.second);
    AnimationDebuggingInfo& debug = std::get<AnimationDebuggingInfo>(anim.second);

    _ryuAnimations.RegisterAnimation(anim.first, params.sheet.c_str(), params.rows, params.columns, params.startIndexOnSheet, params.frames, params.anchor);
    _ryuAnimations.SetHitboxEvents(anim.first, hitboxSheet.c_str(), frameData); 

    std::string spriteSheetFile = params.sheet;

    std::function<void()> ModFrameData = [animName, hitboxSheet, spriteSheetFile, &frameData, &debug]()
    {
      if (ImGui::CollapsingHeader(animName.c_str()))
      {
        ImGui::BeginGroup();
        ImGui::InputInt("Start Up Frames", &frameData.startUp);
        ImGui::InputInt("Active Frames", &frameData.active);
        ImGui::InputInt("Recovery Frames", &frameData.recover);
        ImGui::EndGroup();

        ImGui::BeginGroup();
        ImGui::InputInt("On Hit Frame Advantage", &frameData.onHitAdvantage);
        ImGui::InputInt("On Block Frame Advantage", &frameData.onBlockAdvantage);
        ImGui::EndGroup();

        ImGui::BeginGroup();
        ImGui::InputInt("Damage", &frameData.damage);
        ImGui::InputFloat2("Knockback Vector", &frameData.knockback.x, 1);
        ImGui::InputInt("Hit stop", &frameData.hitstop);
        ImGui::EndGroup();

        int& frame = debug.frame;
        _ryuAnimations.GetAnimation(animName)->DisplayDebugFrame(128, frame);
        
        int nFrames = _ryuAnimations.GetAnimation(animName)->GetFrameCount();
        ImGui::BeginGroup();
        if(ImGui::Button("Back"))
        {
          frame = frame == 0 ? nFrames - 1 : frame - 1;
        }
        ImGui::SameLine();
        if(ImGui::Button("Forward"))
        {
          frame = (frame + 1) % nFrames;
        }
        ImGui::SameLine();
        ImGui::Text("%d", frame);
        ImGui::EndGroup();

        if (ImGui::Button("Set Frame Data"))
        {
          _ryuAnimations.SetHitboxEvents(animName, hitboxSheet.c_str(), frameData);
        }
      }
    };

    GUIController::Get().AddImguiWindowFunction("Ryu Attack Animations", ModFrameData);
  }
}