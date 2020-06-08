#include "AssetManagement/StaticAssets/StaticAssetUtils.h"
#include "DebugGUI/GUIController.h"

void StaticAssetUtils::LoadAnimations(std::unordered_map<std::string, AnimationInfo>& normalAnimations, 
  std::unordered_map<std::string, std::tuple<AnimationInfo, FrameData, std::string, AnimationDebuggingInfo>>& attackAnimations,
  AnimationCollection& collection)
{
  //hack cause i suck
  auto it = normalAnimations.find("Idle");
  collection.RegisterAnimation("Idle", SpriteSheet(it->second.sheet.src.c_str(), it->second.sheet.rows, it->second.sheet.columns), it->second.startIndexOnSheet, it->second.frames, it->second.anchor);

  for (auto& anim : normalAnimations)
  {
    AnimationInfo& params = anim.second;
    collection.RegisterAnimation(anim.first, SpriteSheet(anim.second.sheet.src.c_str(), anim.second.sheet.rows, anim.second.sheet.columns), params.startIndexOnSheet, params.frames, params.anchor);
  }
  for (auto& anim : attackAnimations)
  {
    std::string animName = anim.first;
    std::string hitboxSheet = std::get<2>(anim.second);
    AnimationInfo& params = std::get<AnimationInfo>(anim.second);
    FrameData& frameData = std::get<1>(anim.second);
    AnimationDebuggingInfo& debug = std::get<AnimationDebuggingInfo>(anim.second);

    collection.RegisterAnimation(anim.first, SpriteSheet(params.sheet.src.c_str(), params.sheet.rows, params.sheet.columns), params.startIndexOnSheet, params.frames, params.anchor);
    collection.SetHitboxEvents(anim.first, hitboxSheet.c_str(), frameData);

    std::string spriteSheetFile = params.sheet.src;

    std::function<void()> ModFrameData = [animName, hitboxSheet, spriteSheetFile, &frameData, &debug, &collection]()
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
        Animation::ImGuiDisplayParams imParams = collection.GetAnimation(animName)->GetUVCoordsForFrame(128, frame);
        ImGui::Image((void*)(intptr_t)imParams.ptr, ImVec2(imParams.displaySize.x, imParams.displaySize.y),
          ImVec2(imParams.uv0.x, imParams.uv0.y), ImVec2(imParams.uv1.x, imParams.uv1.y));

        int nFrames = collection.GetAnimation(animName)->GetFrameCount();
        ImGui::BeginGroup();
        if (ImGui::Button("Back"))
        {
          frame = frame == 0 ? nFrames - 1 : frame - 1;
        }
        ImGui::SameLine();
        if (ImGui::Button("Forward"))
        {
          frame = (frame + 1) % nFrames;
        }
        ImGui::SameLine();
        ImGui::Text("%d", frame);
        ImGui::EndGroup();

        if (ImGui::Button("Set Frame Data"))
        {
          collection.SetHitboxEvents(animName, hitboxSheet.c_str(), frameData);
        }
      }
    };

    GUIController::Get().AddImguiWindowFunction("Ryu Attack Animations", ModFrameData);
  }
}
