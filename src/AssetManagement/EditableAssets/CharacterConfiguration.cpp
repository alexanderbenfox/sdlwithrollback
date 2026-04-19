#include "AssetManagement/EditableAssets/CharacterConfiguration.h"
#include "AssetManagement/EditableAssets/AssetLibraryImpl.h"
#include "DebugGUI/GUIController.h"
#include "AssetManagement/IAnimation.h"
#include "AssetManagement/Animation.h"

#include "Managers/AnimationCollectionManager.h"
#include "Managers/GameManagement.h"

#include "AssetManagement/EditableAssets/Editor/AnimationEditor.h"

#include "DebugGUI/DisplayImage.h"
#include "DebugGUI/EditorRect.h"

#include "Core/FSM/FighterStateTable.h"

//______________________________________________________________________________
// HitboxEditor — now animation-type-agnostic
//______________________________________________________________________________

void HitboxEditor::OpenEditor(IAnimation* anim, ActionAsset& data)
{
  GameManager::Get().TriggerEndOfFrame([this, anim, &data]()
    {
      static int frame = 0;

      if (GUIController::Get().HasWindow("View Hitboxes"))
      {
        GUIController::Get().RemoveImguiWindowFunction("View Hitboxes", 0);
        CommitRectChange(anim, frame, data);
      }

      frame = 0;
      ChangeDisplay(anim, frame, data);

      GUIController::Get().AddImguiWindowFunction("View Hitboxes", "View",
        [this, anim, &data]()
        {
          int nFrames = anim->GetFrameCount();
          ImGui::BeginGroup();
          if (ImGui::Button("Back"))
          {
            CommitRectChange(anim, frame, data);
            frame = frame == 0 ? nFrames - 1 : frame - 1;
            ChangeDisplay(anim, frame, data);
          }
          ImGui::SameLine();
          if (ImGui::Button("Forward"))
          {
            CommitRectChange(anim, frame, data);
            frame = (frame + 1) % nFrames;
            ChangeDisplay(anim, frame, data);
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

void HitboxEditor::ChangeDisplay(IAnimation* anim, int frame, ActionAsset& data)
{
  frameDisplay = anim->GetEditorPreview(512, frame);
  displayRect = EditorRect(frameDisplay.displaySize);

  int evtFrame = anim->GetFrameIndexOffset(frame);
  if (evtFrame < static_cast<int>(data.eventData.size()))
  {
    Rect<double>& hitbox = data.eventData[evtFrame].hitbox;
    Vector2<double> srcSize = anim->GetFrameSourceSize(frame);
    displayRect.Import(hitbox, srcSize);
  }
}

void HitboxEditor::CommitRectChange(IAnimation* anim, int frame, ActionAsset& data)
{
  int evtFrame = anim->GetFrameIndexOffset(frame);
  if (evtFrame < static_cast<int>(data.eventData.size()))
  {
    if (displayRect.UserDataExists())
    {
      Vector2<double> srcSize = anim->GetFrameSourceSize(frame);
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
// CharacterConfiguration
//______________________________________________________________________________

CharacterConfiguration::CharacterConfiguration(const std::string& pathToResourceFolder) : _resourcePath(pathToResourceFolder)
{
  FilePath p(pathToResourceFolder);
  _characterIdentifier = p.GetLast();

  // should move sprite sheets out of character folders - remove this once that is done
  LoadAssetFile("spritesheets.json", ResourceManager::Get().gSpriteSheets);
  LoadAssetFile("animations.json", _animations);
  LoadAssetFile("actions.json", _actions);

  // Propagate renames: animation/action names must stay in sync with each other
  // and with the state table's animationName fields (used as lookup key for both)
  auto updateStateTable = [this](const std::string& oldName, const std::string& newName)
  {
    FighterStateTable::StateArray* states = FighterStateTable::Get().GetMutableTable(_characterIdentifier);
    if (!states) return;
    for (auto& state : *states)
    {
      if (state.animationName == oldName)
        state.animationName = newName;
    }
  };

  _animations.SetOnRenameCallback([this, updateStateTable](const std::string& oldName, const std::string& newName)
  {
    updateStateTable(oldName, newName);
    _actions.RenameKey(oldName, newName);
  });

  _actions.SetOnRenameCallback([this, updateStateTable](const std::string& oldName, const std::string& newName)
  {
    updateStateTable(oldName, newName);
    _animations.RenameKey(oldName, newName);
  });

  AddCharacterDisplay();
}

//______________________________________________________________________________
void CharacterConfiguration::AddCharacterDisplay()
{
  GUIController::Get().AddImguiWindowFunction("Characters", _characterIdentifier, "Assets", [this]()
  {
    if (ImGui::BeginTabBar("CharacterTabs"))
    {
      if (ImGui::BeginTabItem("Animations"))
      {
        DisplayAnimationsTab();
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem("Actions"))
      {
        DisplayActionsTab();
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem("States"))
      {
        DisplayStatesTab();
        ImGui::EndTabItem();
      }
      ImGui::EndTabBar();
    }
  });
}

//______________________________________________________________________________
void CharacterConfiguration::DisplayAnimationsTab()
{
  _animations.DisplayInGUI();

  if (ImGui::Button("Save Animations"))
    SaveAssetFile("animations.json", _animations);

  ImGui::SameLine();
  if (ImGui::Button("Reload Animation Collection"))
    AnimationCollectionManager::Get().ReloadAnimationCollection(_characterIdentifier, *this);
}

//______________________________________________________________________________
void CharacterConfiguration::DisplayActionsTab()
{
  _actions.DisplayInGUI();

  if (ImGui::Button("Save Actions"))
    SaveAssetFile("actions.json", _actions);

  ImGui::Separator();

  // Action editing — select an action to edit hitboxes and frame data
  std::vector<std::string> actionNames;
  for (const auto& action : _actions.GetLibrary())
    actionNames.push_back(action.first);

  ImGui::Text("Modify Action: ");
  ImGui::SameLine();
  DropDown::DisplayList(actionNames, _selectedAction);

  if (!_selectedAction.empty())
  {
    ActionAsset& data = _actions.GetModifiable(_selectedAction);
    AnimationCollection& collection = GAnimArchive.GetCollection(GAnimArchive.GetCollectionID(_characterIdentifier));
    IAnimation* anim = collection.GetAnimation(_selectedAction);

    if (anim)
    {
      if (ImGui::Button("View/Edit Hitboxes"))
        _hitboxEditor.OpenEditor(anim, data);

      if (ImGui::Button("Set Frame Data"))
      {
        // SetAnimationEvents still needs the concrete Animation* for event generation
        Animation* spriteAnim = collection.GetSpriteAnimation(_selectedAction);
        if (spriteAnim)
          collection.SetAnimationEvents(_selectedAction, data.eventData, data.frameData);
      }
    }
    else
    {
      ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "No animation found for '%s'", _selectedAction.c_str());
    }
  }
}

//______________________________________________________________________________
void CharacterConfiguration::DisplayStatesTab()
{
  _stateVisualizer.Display(_characterIdentifier);
}
