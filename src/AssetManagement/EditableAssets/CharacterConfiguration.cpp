#include "AssetManagement/EditableAssets/CharacterConfiguration.h"
#include "DebugGUI/GUIController.h"
#include "AssetManagement/Animation.h"

#include "Managers/GameManagement.h"

#include "AssetManagement/EditableAssets/Editor/AnimationEditor.h"

#include "DebugGUI/DisplayImage.h"
#include "DebugGUI/EditorRect.h"

//______________________________________________________________________________
void HitboxEditor::OpenEditor(Animation* anim, ActionAsset& data, const std::string& spriteSheetID)
{
  const SpriteSheet& sheet = AssetLibrary<SpriteSheet>::Get(spriteSheetID);
  GameManager::Get().TriggerEndOfFrame([this, anim, &sheet, &data]()
    {
      static int frame = 0;

      if (GUIController::Get().HasWindow("View Hitboxes"))
      {
        GUIController::Get().RemoveImguiWindowFunction("View Hitboxes", 0);
        CommitRectChange(anim, frame, data, sheet);
      }

      frame = 0;
      ChangeDisplay(anim, frame, data, sheet);

      GUIController::Get().AddImguiWindowFunction("View Hitboxes", "View",
        [this, anim, &sheet, &data]()
        {

          int nFrames = anim->GetFrameCount();
          ImGui::BeginGroup();
          if (ImGui::Button("Back"))
          {
            CommitRectChange(anim, frame, data, sheet);
            frame = frame == 0 ? nFrames - 1 : frame - 1;
            ChangeDisplay(anim, frame, data, sheet);
          }
          ImGui::SameLine();
          if (ImGui::Button("Forward"))
          {
            CommitRectChange(anim, frame, data, sheet);
            frame = (frame + 1) % nFrames;
            ChangeDisplay(anim, frame, data, sheet);
          }
          ImGui::SameLine();
          ImGui::Text("%d", frame);
          ImGui::EndGroup();

          ShowHitboxEditor();
        });
      GUIController::Get().GetWindow("View Hitboxes").OpenWindow();
    });
}


void HitboxEditor::ChangeDisplay(Animation* anim, int frame, ActionAsset& data, const SpriteSheet& sheet)
{
  frameDisplay = anim->GetGUIDisplayImage(512, frame);
  displayRect = EditorRect(frameDisplay.displaySize);

  int evtFrame = anim->AnimFrameToSheet(frame);
  if (evtFrame < data.eventData.size())
  {
    Rect<double>& hitbox = data.eventData[evtFrame].hitbox;

    double srcW = static_cast<double>(sheet.sheetSize.x) / static_cast<double>(sheet.columns);
    double srcH = static_cast<double>(sheet.sheetSize.y) / static_cast<double>(sheet.rows);
    Vector2<double> srcSize(srcW, srcH);

    displayRect.Import(hitbox, srcSize);
  }
}

void HitboxEditor::CommitRectChange(Animation* anim, int frame, ActionAsset& data, const SpriteSheet& sheet)
{
  int evtFrame = anim->AnimFrameToSheet(frame);
  if (evtFrame < data.eventData.size())
  {
    if (displayRect.UserDataExists())
    {
      double srcW = static_cast<double>(sheet.sheetSize.x) / static_cast<double>(sheet.columns);
      double srcH = static_cast<double>(sheet.sheetSize.y) / static_cast<double>(sheet.rows);
      Vector2<double> srcSize(srcW, srcH);

      data.eventData[evtFrame].hitbox = displayRect.Export(srcSize);
    }
    else
    {
      data.eventData[evtFrame].hitbox = Rect<double>();
    }
  }
}

void HitboxEditor::ShowHitboxEditor()
{
  ImGui::BeginChild("Box Editor");
  Vector2<float> position = frameDisplay.Show();
  displayRect.DisplayAtPosition(position);
  ImGui::EndChild();
}

//______________________________________________________________________________
CharacterConfiguration::CharacterConfiguration(const std::string& pathToResourceFolder) : _resourcePath(pathToResourceFolder)
{
  FilePath p(pathToResourceFolder);
  _characterIdentifier = p.GetLast();

  //LoadAssetFile("spritesheets.json", _spriteSheets);
  // should move sprite sheets out of character folders - remove this once that is done
  FilePath path = _resourcePath;
  path.Append("spritesheets.json");
  JsonFile json(path.GetPath());
  AssetLibrary<SpriteSheet>::LoadJsonData(json);

  LoadAssetFile("animations.json", _animations);
  LoadAssetFile("actions.json", _actions);

  AddCharacterDisplay();
}

void CharacterConfiguration::ReloadActionDebug(const std::string& actionName, ActionAsset& data)
{
  // hitbox editor window
  static HitboxEditor hitboxEditor;

  AnimationCollection& collection = GAnimArchive.GetCollection(GAnimArchive.GetCollectionID(_characterIdentifier));

  if (ImGui::Button("View/Edit Hitboxes"))
  {
    hitboxEditor.OpenEditor(collection.GetAnimation(actionName), data, _animations[actionName].sheetName);
  }

  if (ImGui::Button("Set Frame Data"))
  {
    // generate event list based on new frame data
    collection.SetAnimationEvents(actionName, data.eventData, data.frameData);
  }
}

//______________________________________________________________________________
void CharacterConfiguration::AddCharacterDisplay()
{
  GUIController::Get().AddImguiWindowFunction("Characters", _characterIdentifier, "Assets", [this]()
    {
      unsigned int counter = 0;
      /*if (ImGui::CollapsingHeader("Sprite Sheets"))
      {
        ImGui::Text("Sprite Sheets");
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
      }*/

      if (ImGui::CollapsingHeader("Animations"))
      {
        ImGui::Text("Animations");
        for (auto& item : _animations)
        {
          const int fieldHeight = 25;
          std::string name = "##item:" + std::to_string(counter++);
          ImGui::BeginChild(name.c_str(), ImVec2(500, 6 * fieldHeight), true);
          ImGui::Text("%s", item.first.c_str());
          item.second.DisplayInEditor();
          if (ImGui::Button("Set Anchor In-Game"))
          {
            GAnimArchive.GetCollection(GAnimArchive.GetCollectionID(_characterIdentifier)).GetAnimation(item.first)->anchorPoint = std::make_pair(item.second.anchor, item.second.GetAnchorPosition());
          }
          ImGui::EndChild();
        }

        if (ImGui::Button("Auto Generate Offset For Animations "))
        {
          for (auto& item : _animations)
          {
            const SpriteSheet& animSpriteSheet = AssetLibrary<SpriteSheet>::Get(item.second.sheetName);
            Vector2<int> anchorPos = AnimationAsset::GenerateAnchorPoint(item.second.anchor, animSpriteSheet, item.second.startIndexOnSheet, false);
            item.second.anchorPoints[(int)item.second.anchor].Import(anchorPos, animSpriteSheet.frameSize);
            GAnimArchive.GetCollection(GAnimArchive.GetCollectionID(_characterIdentifier)).GetAnimation(item.first)->anchorPoint = std::make_pair(item.second.anchor, item.second.GetAnchorPosition());
          }
        }

        if (ImGui::CollapsingHeader("Create New Animation"))
        {
          animName.DisplayEditable("New Animation Name");
          newAnimation.DisplayInEditor();
          if (ImGui::Button("Add Anim"))
          {
            _animations.insert(std::make_pair((std::string)animName, newAnimation));
          }
        }


        if (ImGui::Button("Save Animations"))
        {
          SaveAssetFile("animations.json", _animations);
        }
      }

      if (ImGui::CollapsingHeader("Actions"))
      {
        ImGui::Text("Actions");
        for (auto& item : _actions)
        {
          if (ImGui::CollapsingHeader(item.first.c_str()))
          {
            item.second.DisplayInEditor();
            ReloadActionDebug(item.first, item.second);
          }
        }

        if (ImGui::CollapsingHeader("Create New Action"))
        {
          actionName.DisplayEditable("New Action Name");
          newAction.DisplayInEditor();
          if (ImGui::Button("Add Action"))
          {
            _actions.insert(std::make_pair((std::string)actionName, newAction));
          }
        }

        if (ImGui::Button("Save Actions"))
        {
          SaveAssetFile("actions.json", _actions);
        }
      }
    });
}
