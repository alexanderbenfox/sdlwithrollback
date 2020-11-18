#include "AssetManagement/EditableAssets/CharacterConfiguration.h"
#include "DebugGUI/GUIController.h"
#include "AssetManagement/Animation.h"

#include "Managers/AnimationCollectionManager.h"
#include "Managers/GameManagement.h"

#include "AssetManagement/EditableAssets/Editor/AnimationEditor.h"

#include "DebugGUI/DisplayImage.h"
#include "DebugGUI/EditorRect.h"

//______________________________________________________________________________
void HitboxEditor::OpenEditor(Animation* anim, ActionAsset& data, const std::string& spriteSheetID, const std::string& subSheet)
{
  const SpriteSheet::Section& sheet = ResourceManager::Get().gSpriteSheets.Get(spriteSheetID).GetSubSection(subSheet);
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

          ImGui::BeginGroup();
          data.DisplayInEditor();
          ImGui::EndGroup();
        });
      GUIController::Get().GetWindow("View Hitboxes").OpenWindow();
    });
}


void HitboxEditor::ChangeDisplay(Animation* anim, int frame, ActionAsset& data, const SpriteSheet::Section& sheet)
{
  frameDisplay = anim->GetGUIDisplayImage(512, frame);
  displayRect = EditorRect(frameDisplay.displaySize);

  int evtFrame = anim->AnimFrameToSheet(frame);
  if (evtFrame < data.eventData.size())
  {
    Rect<double>& hitbox = data.eventData[evtFrame].hitbox;

    double srcW = static_cast<double>(sheet.frameSize.x);
    double srcH = static_cast<double>(sheet.frameSize.y);
    Vector2<double> srcSize(srcW, srcH);

    displayRect.Import(hitbox, srcSize);
  }
}

void HitboxEditor::CommitRectChange(Animation* anim, int frame, ActionAsset& data, const SpriteSheet::Section& sheet)
{
  int evtFrame = anim->AnimFrameToSheet(frame);
  if (evtFrame < data.eventData.size())
  {
    if (displayRect.UserDataExists())
    {
      double srcW = static_cast<double>(sheet.frameSize.x);
      double srcH = static_cast<double>(sheet.frameSize.y);
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

  // should move sprite sheets out of character folders - remove this once that is done
  LoadAssetFile("spritesheets.json", ResourceManager::Get().gSpriteSheets);
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
    hitboxEditor.OpenEditor(collection.GetAnimation(actionName), data, _animations.GetLibrary().at(actionName).sheetName, _animations.GetLibrary().at(actionName).subSheetName);
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
      if (ImGui::CollapsingHeader("Animations"))
      {
        _animations.DisplayInGUI();
        if (ImGui::Button("Save Animations"))
          SaveAssetFile("animations.json", _animations);
      }
      if (ImGui::CollapsingHeader("Actions"))
      {
        _actions.DisplayInGUI();
        if (ImGui::Button("Save Actions"))
          SaveAssetFile("actions.json", _actions);
      }

      static std::string selectedAction = "";
      std::vector<std::string> actionNames;
      for (const auto& action : _actions.GetLibrary())
      {
        actionNames.push_back(action.first);
      }

      ImGui::BeginGroup();
      ImGui::Text("Modify Action: ");
      ImGui::SameLine();
      DropDown::DisplayList(actionNames, selectedAction, [this]()
      {
        if (!selectedAction.empty())
        {
          ReloadActionDebug(selectedAction, _actions.GetModifiable(selectedAction));
        }
      });
      ImGui::EndGroup();

      if (ImGui::Button("Reload Animation Collection"))
      {
        AnimationCollectionManager::Get().ReloadAnimationCollection(_characterIdentifier, *this);
      }
    });
}
