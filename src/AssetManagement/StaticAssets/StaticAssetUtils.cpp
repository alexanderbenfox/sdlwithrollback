#include "AssetManagement/StaticAssets/StaticAssetUtils.h"
#include "DebugGUI/GUIController.h"

#include <fstream>
#include <json/json.h>

std::string StaticAssetUtils::jsonWriteOutLocation = "..\\resources\\moveset_data\\ryu_moves.json";
bool StaticAssetUtils::loadFromFile = true;

void StaticAssetUtils::LoadAnimations(std::unordered_map<std::string, AnimationInfo>& normalAnimations, 
  std::unordered_map<std::string, std::tuple<AnimationInfo, FrameData, std::string, AnimationDebuggingInfo>>& attackAnimations,
  AnimationCollection& collection)
{
  const size_t fileCharacterSize = 256;
  jsonWriteOutLocation.resize(fileCharacterSize);

  std::function<void()> f = [fileCharacterSize]() {
    ImGui::BeginGroup();
    ImGui::InputText("Write out Json location", StaticAssetUtils::jsonWriteOutLocation.data(), fileCharacterSize);
    ImGui::EndGroup();
  };

  GUIController::Get().AddImguiWindowFunction("Ryu Attack Animations", f);

  //hack cause i suck
  auto it = normalAnimations.find("Idle");
  collection.RegisterAnimation("Idle", SpriteSheet(it->second.sheet.src.c_str(), it->second.sheet.rows, it->second.sheet.columns), it->second.startIndexOnSheet, it->second.frames, it->second.anchor);

  for (auto& anim : normalAnimations)
  {
    AnimationInfo& params = anim.second;
    collection.RegisterAnimation(anim.first, SpriteSheet(anim.second.sheet.src.c_str(), anim.second.sheet.rows, anim.second.sheet.columns), params.startIndexOnSheet, params.frames, params.anchor);
  }

  if (StaticAssetUtils::loadFromFile)
  {
    collection.LoadCollectionFromJson("..\\resources\\moveset_data\\ryu_spritesheets.json", jsonWriteOutLocation);
  }
  else
  {
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

      std::function<void()> ModFrameData = [animName, hitboxSheet, spriteSheetFile, &frameData, &debug, &params, &collection]()
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

          ImGui::Text("Write to json");
          if (ImGui::Button("Write Out Data"))
          {
            std::fstream dataFile;
            dataFile.open(jsonWriteOutLocation, std::fstream::in | std::fstream::out);
            Json::Value moveListObj;

            if (dataFile.is_open())
            {
              dataFile >> moveListObj;

              // remove existing entry for this move if it exists
              if (!moveListObj.isNull())
              {
                moveListObj.removeMember(animName);
              }
              dataFile.close();
            }

            auto& item = moveListObj[animName];

            AttackAnimationData data;
            data.frameData = frameData;
            data.loadingInfo = params;

            if (auto* evtList = collection.GetEventList(animName))
            {
              Animation* a = collection.GetAnimation(animName);
              for (const auto& hb : a->hitboxes)
              {
                data.eventData.push_back(AnimationActionEventData());
                data.eventData.back().hitbox = hb;
              }
            }

            // append new data to existing json
            data.Write(item);

            dataFile.open(jsonWriteOutLocation, std::fstream::out);
            dataFile << moveListObj.toStyledString() << std::endl;
            dataFile.close();
          }
        }
      };
      GUIController::Get().AddImguiWindowFunction("Ryu Attack Animations", ModFrameData);
    }

    
  }
}
