#include "AssetManagement/EditableAssets/CharacterConfiguration.h"
#include "DebugGUI/GUIController.h"
#include "AssetManagement/Animation.h"

void CharacterConfiguration::CreateDebugMenuActions(AnimationCollection* collection)
{
  for (auto& action : _actions)
  {
    std::string animName = action.first;
    ActionAsset& data = action.second;

    std::function<void()> ModFrameData = [this, animName, &data, collection]()
    {
      if (ImGui::CollapsingHeader(animName.c_str()))
      {
        static int frame = 0;

        data.frameData.DisplayEditableData();
        //Animation::ImGuiDisplayParams imParams = collection.GetAnimation(animName)->GetUVCoordsForFrame(128, frame);
        //ImGui::Image((void*)(intptr_t)imParams.ptr, ImVec2(imParams.displaySize.x, imParams.displaySize.y),
        //  ImVec2(imParams.uv0.x, imParams.uv0.y), ImVec2(imParams.uv1.x, imParams.uv1.y));

        // show hitbox and animation frame if it is there
        DisplayFrameHitbox(collection->GetAnimation(animName), data, frame);


        int nFrames = _animations[animName].frames;
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

        /*ImGui::BeginGroup();
        if (ImGui::CollapsingHeader("FRAME DATA"))
        {
          for(int i = 0; i < data.eventData.size(); i++)
          {
            std::string str = std::to_string(i);
            auto mvx = str + "mvx";
            auto mvy = str + "mvy";
            ImGui::Text("%d", i);
            ImGui::SameLine();
            ImGui::InputFloat(mvx.c_str(), &data.eventData[i].movement.x, 1, 10, 1);
            ImGui::SameLine();
            ImGui::InputFloat(mvy.c_str(), &data.eventData[i].movement.y, 1, 10, 1);
          }
        }
        ImGui::EndGroup();*/


        if (ImGui::Button("Set Frame Data"))
        {
          // generate event list based on new frame data
          //collection.SetAnimationEvents(animName, collection.GetAnimation(animName)->animationEvents, data.frameData);
          collection->SetAnimationEvents(animName, data.eventData, data.frameData);
        }

        ImGui::Text("Write to json");
        if (ImGui::Button("Write Out Data"))
        {
          FilePath path = _resourcePath;
          path.Append("actions.json");
          JsonFile json(path.GetPath());
          json.OverwriteMemberInFile(animName, data);
        }
      }
    };
    GUIController::Get().AddImguiWindowFunction("Ryu Character Data", "Attack Animations", ModFrameData);
  }
}

//______________________________________________________________________________
void CharacterConfiguration::DisplayFrameHitbox(Animation* animation, ActionAsset& data, int animationFrame)
{
  int evtFrame = animation->AnimFrameToSheet(animationFrame);
  if (evtFrame < data.eventData.size())
  {
    Rect<double> hitbox = data.eventData[evtFrame].hitbox;
    if (hitbox.Area() != 0)
    {
      const SpriteSheet& sheet = _spriteSheets[_animations[data.animationName].sheetName];
      double srcW = static_cast<double>(sheet.sheetSize.x) / static_cast<double>(sheet.columns);
      double srcH = static_cast<double>(sheet.sheetSize.y) / static_cast<double>(sheet.rows);

      Animation::ImGuiDisplayParams imParamsHitbox = animation->GetUVCoordsForFrame(512, animationFrame);
      Vector2<double> displaySize = imParamsHitbox.displaySize;

      ImGui::BeginChild("Animation Frame Display", ImVec2(displaySize.x, displaySize.y), false);
      ImVec2 currPos = ImGui::GetWindowPos();

      ImGui::Image((void*)(intptr_t)imParamsHitbox.ptr, ImVec2(displaySize.x, displaySize.y), ImVec2(imParamsHitbox.uv0.x, imParamsHitbox.uv0.y), ImVec2(imParamsHitbox.uv1.x, imParamsHitbox.uv1.y));

      Vector2<double> drawBeg(hitbox.beg.x / srcW * displaySize.x, hitbox.beg.y / srcH * displaySize.y);
      Vector2<double> drawEnd(hitbox.end.x / srcW * displaySize.x, hitbox.end.y / srcH * displaySize.y);

      ImDrawList* draws = ImGui::GetWindowDrawList();
      draws->AddRect(ImVec2(currPos.x + drawBeg.x, currPos.y + drawBeg.y), ImVec2(currPos.x + drawEnd.x, currPos.y + drawEnd.y), IM_COL32(255, 0, 0, 255));
      ImGui::EndChild();
    }
  }
}
