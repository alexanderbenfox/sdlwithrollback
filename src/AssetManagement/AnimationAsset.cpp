#include "Rendering/GLTexture.h"
#include "AssetManagement/AnimationAsset.h"
#include "GameManagement.h"
#include "AssetManagement/StaticAssets/CharacterConfig.h"
#include "DebugGUI/GUIController.h"

AnimationCollection& AnimationAsset::RyuAnimations()
{
  if(!_ryuAnimLoaded)
  {
    LoadRyuAnimations();
    _ryuAnimLoaded = true;
  }
  return _ryuAnimations;
}

bool AnimationAsset::_ryuAnimLoaded = false;

AnimationCollection AnimationAsset::_ryuAnimations;

void AnimationAsset::LoadRyuAnimations()
{
  //hack cause i suck
  auto it = RyuConfig::normalAnimations.find("Idle");
  _ryuAnimations.RegisterAnimation("Idle", SpriteSheet(it->second.sheet.src.c_str(), it->second.sheet.rows, it->second.sheet.columns), it->second.startIndexOnSheet, it->second.frames, it->second.anchor);

  for(auto& anim : RyuConfig::normalAnimations)
  {
    AnimationInfo& params = anim.second;
    _ryuAnimations.RegisterAnimation(anim.first, SpriteSheet(anim.second.sheet.src.c_str(), anim.second.sheet.rows, anim.second.sheet.columns), params.startIndexOnSheet, params.frames, params.anchor);
  }
  for(auto& anim : RyuConfig::attackAnimations)
  {
    std::string animName = anim.first;
    std::string hitboxSheet = std::get<2>(anim.second);
    AnimationInfo& params = std::get<AnimationInfo>(anim.second);
    FrameData& frameData = std::get<1>(anim.second);
    AnimationDebuggingInfo& debug = std::get<AnimationDebuggingInfo>(anim.second);

    _ryuAnimations.RegisterAnimation(anim.first, SpriteSheet(params.sheet.src.c_str(), params.sheet.rows, params.sheet.columns), params.startIndexOnSheet, params.frames, params.anchor);
    _ryuAnimations.SetHitboxEvents(anim.first, hitboxSheet.c_str(), frameData); 

    std::string spriteSheetFile = params.sheet.src;

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
        Animation::ImGuiDisplayParams imParams = _ryuAnimations.GetAnimation(animName)->GetUVCoordsForFrame(128, frame);
        ImGui::Image((void*)(intptr_t)imParams.ptr, ImVec2(imParams.displaySize.x, imParams.displaySize.y),
          ImVec2(imParams.uv0.x, imParams.uv0.y), ImVec2(imParams.uv1.x, imParams.uv1.y));

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