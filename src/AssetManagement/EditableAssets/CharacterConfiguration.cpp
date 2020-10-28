#include "AssetManagement/EditableAssets/CharacterConfiguration.h"
#include "DebugGUI/GUIController.h"
#include "AssetManagement/Animation.h"

#include "Managers/GameManagement.h"

#include "AssetManagement/EditableAssets/Editor/AnimationEditor.h"

CharacterConfiguration::CharacterConfiguration(const std::string& pathToResourceFolder) : _resourcePath(pathToResourceFolder)
{
  FilePath p(pathToResourceFolder);
  _characterIdentifier = p.GetLast();

  LoadAssetFile("spritesheets.json", _spriteSheets);

  // remove this once the data is saved in the json...
  for (auto& item : _spriteSheets)
  {
    item.second.GenerateSheetInfo();
  }

  LoadAssetFile("animations.json", _animations);

  LoadAssetFile("actions.json", _actions);
  // get the "sheetToAnimFrame" data and store it so this doesnt have to be tied to the collection
  /*for (auto& item : _actions)
  {
    _actionEventLookupers[item.first] = AnimationEventHelper::ParseAnimationEventList(item.second.eventData, item.second.frameData, _animations[item.first].frames);
  }*/

  AddCharacterDisplay();
}

void CharacterConfiguration::CreateDebugMenuActions(AnimationCollection* collection)
{
  std::function<void()> ModFrameData = [this, collection]()
  {
    for (auto& action : _actions)
    {
      std::string animName = action.first;
      ActionAsset& data = action.second;
      if (ImGui::CollapsingHeader(animName.c_str()))
      {
        // This just shows frame data right now
        data.DisplayInEditor();

        if (ImGui::Button("View/Edit Hitboxes"))
        {
          GameManager::Get().TriggerEndOfFrame([this, animName, collection, &data]()
            {
              if (GUIController::Get().HasWindow("View Hitboxes"))
                GUIController::Get().RemoveImguiWindowFunction("View Hitboxes", 0);

              GUIController::Get().AddImguiWindowFunction("View Hitboxes", "View",
                [this, animName, collection, &data]()
                {
                  static int frame = 0;
                  int nFrames = collection->GetAnimation(animName)->GetFrameCount();
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

                  DisplayFrameHitbox(animName, collection->GetAnimation(animName), data, frame);
                });
              GUIController::Get().GetWindow("View Hitboxes").OpenWindow();
            });
        }



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
    }
  };
  GUIController::Get().AddImguiWindowFunction("Ryu Character Data", "Attack Animations", ModFrameData);
}

//______________________________________________________________________________
void CharacterConfiguration::DisplayFrameHitbox(const std::string& animName, Animation* animation, ActionAsset& data, int animationFrame)
{
  int evtFrame = animation->AnimFrameToSheet(animationFrame);
  if (evtFrame < data.eventData.size())
  {
    Animation::ImGuiDisplayParams displayParams = animation->GetUVCoordsForFrame(512, animationFrame);
    Vector2<double> displaySize = displayParams.displaySize;

    ImGui::BeginChild("Animation Frame Display", ImVec2(displaySize.x, displaySize.y), false);
    ImVec2 currPos = ImGui::GetWindowPos();

    ImGui::Image((void*)(intptr_t)displayParams.ptr, ImVec2(displaySize.x, displaySize.y), ImVec2(displayParams.uv0.x, displayParams.uv0.y), ImVec2(displayParams.uv1.x, displayParams.uv1.y));

    Rect<double>& hitbox = data.eventData[evtFrame].hitbox;

    // check if an edit is made
    // if no hitbox exists and there was a click, create a small hb at the click pos
    if (ImGui::GetIO().MouseClicked[0])
    {
      ImVec2 cursor = ImGui::GetMousePos();
      Vector2<double> cursorPos(cursor.x - currPos.x, cursor.y - currPos.y);

      if (!(cursorPos.x < 0 || cursorPos.y < 0 || cursorPos.x > displaySize.x || cursorPos.y > displaySize.y))
      {
        const SpriteSheet& sheet = _spriteSheets[_animations[animName].sheetName];
        double srcW = static_cast<double>(sheet.sheetSize.x) / static_cast<double>(sheet.columns);
        double srcH = static_cast<double>(sheet.sheetSize.y) / static_cast<double>(sheet.rows);
        Vector2<double> srcSize(srcW, srcH);

        ActionEditor::EditHitboxExtentsInDisplay(displaySize, srcSize, cursorPos, hitbox);
      }
    }

    // draw hitbox if it exists and check for clicks
    if (hitbox.Area() != 0)
    {
      const SpriteSheet& sheet = _spriteSheets[_animations[animName].sheetName];
      double srcW = static_cast<double>(sheet.sheetSize.x) / static_cast<double>(sheet.columns);
      double srcH = static_cast<double>(sheet.sheetSize.y) / static_cast<double>(sheet.rows);

      Vector2<double> drawBeg(hitbox.beg.x / srcW * displaySize.x, hitbox.beg.y / srcH * displaySize.y);
      Vector2<double> drawEnd(hitbox.end.x / srcW * displaySize.x, hitbox.end.y / srcH * displaySize.y);

      ImDrawList* draws = ImGui::GetWindowDrawList();
      draws->AddRect(ImVec2(currPos.x + drawBeg.x, currPos.y + drawBeg.y), ImVec2(currPos.x + drawEnd.x, currPos.y + drawEnd.y), IM_COL32(255, 0, 0, 255));
    }

    ImGui::EndChild();
  }
}

//______________________________________________________________________________
void CharacterConfiguration::AddCharacterDisplay()
{
  GUIController::Get().AddImguiWindowFunction("Characters", _characterIdentifier, "Assets", [this]()
    {
      ImGui::Text("Sprite Sheets");
      unsigned int counter = 0;
      for (auto& item : _spriteSheets)
      {
        std::string name = "##item:" + std::to_string(counter++);
        ImGui::BeginChild(name.c_str(), ImVec2(500, 70), true);
        item.second.DisplayInEditor();
        ImGui::EndChild();
      }
      ImGui::Text("New Sprite Sheet");
      newSheetName.DisplayEditable("Sprite Sheet Label");
      newSheet.DisplayInEditor();
      if (ImGui::Button("Add SS"))
      {
        _spriteSheets.insert(std::make_pair((std::string)newSheetName, newSheet));
      }

      if (ImGui::Button("Save SS"))
      {
        SaveAssetFile("spritesheets.json", _spriteSheets);
      }

      ImGui::Text("Animations");
      for (auto& item : _animations)
      {
        std::string name = "##item:" + std::to_string(counter++);
        ImGui::BeginChild(name.c_str(), ImVec2(500, 70), true);
        ImGui::Text("%s", item.first.c_str());
        item.second.DisplayInEditor();
        ImGui::EndChild();
      }
      ImGui::Text("New Animation");

      animName.DisplayEditable("New Animation Name");
      newAnimation.DisplayInEditor();
      if (ImGui::Button("Add Anim"))
      {
        _animations.insert(std::make_pair((std::string)animName, newAnimation));
      }

      if (ImGui::Button("Save Animations"))
      {
        SaveAssetFile("animations.json", _animations);
      }

      ImGui::Text("Actions");
      for (auto& item : _actions)
      {
        std::string name = "##item:" + std::to_string(counter++);
        ImGui::BeginChild(name.c_str(), ImVec2(500, 70), true);
        ImGui::Text("%s", item.first.c_str());
        item.second.DisplayInEditor();
        ImGui::EndChild();
      }
      ImGui::Text("New Action");

      actionName.DisplayEditable("New Action Name");
      newAction.DisplayInEditor();
      if (ImGui::Button("Add Action"))
      {
        _actions.insert(std::make_pair((std::string)actionName, newAction));
      }

      if (ImGui::Button("Save Actions"))
      {
        SaveAssetFile("actions.json", _actions);
      }
    });
}
