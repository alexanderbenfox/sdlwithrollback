#include "AssetManagement/EditableAssets/CharacterConfiguration.h"
#include "AssetManagement/EditableAssets/AssetLibraryImpl.h"
#include "AssetManagement/IAnimation.h"
#include "AssetManagement/Animation.h"

#include "Managers/AnimationCollectionManager.h"
#include "Managers/GameManagement.h"

#include "DebugGUI/DisplayImage.h"
#include "DebugGUI/EditorRect.h"

#include "Core/FSM/FighterStateTable.h"

//______________________________________________________________________________
// ActionPreview — hitbox editing helpers
//______________________________________________________________________________

void ActionPreview::CommitHitbox()
{
  if (_boundAnim && _boundData)
    CommitHitboxForFrame(_boundAnim, *_boundData);
}

void ActionPreview::EnsureEventDataSize(IAnimation* anim, ActionAsset& data)
{
  // eventData needs one entry per unique sprite sheet frame
  int maxEvtFrame = anim->GetFrameIndexOffset(anim->GetFrameCount() - 1);
  int needed = maxEvtFrame + 1;
  if (static_cast<int>(data.eventData.size()) < needed)
    data.eventData.resize(needed);
}

void ActionPreview::LoadHitboxForFrame(IAnimation* anim, ActionAsset& data)
{
  int displayFrame = anim->PlaysReverse() ? (anim->GetFrameCount() - 1 - _frame) : _frame;
  _preview = anim->GetEditorPreview(_editingHitboxes ? 512 : 256, displayFrame);
  _editRect = EditorRect(_preview.displaySize);

  EnsureEventDataSize(anim, data);

  int evtFrame = anim->GetFrameIndexOffset(_frame);
  _lastEvtFrame = evtFrame;
  _loadedSrcSize = anim->GetFrameSourceSize(_frame);

  Rect<double>& hitbox = data.eventData[evtFrame].hitbox;
  _editRect.Import(hitbox, _loadedSrcSize);
}

void ActionPreview::CommitHitboxForFrame(IAnimation* anim, ActionAsset& data)
{
  if (_lastEvtFrame < 0) return;

  EnsureEventDataSize(anim, data);

  int evtFrame = _lastEvtFrame;
  if (_editRect.UserDataExists())
  {
    data.eventData[evtFrame].hitbox = _editRect.Export(_loadedSrcSize);
    data.eventData[evtFrame].isActive = true;
  }
  else
  {
    data.eventData[evtFrame].hitbox = Rect<double>();
    // Only clear isActive if there's no other active data on this frame
    if (data.eventData[evtFrame].movement.x == 0 && data.eventData[evtFrame].movement.y == 0
        && data.eventData[evtFrame].create.instructions.empty())
      data.eventData[evtFrame].isActive = false;
  }
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
}

//______________________________________________________________________________
void CharacterConfiguration::DisplayEditorTabs()
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
}

//______________________________________________________________________________
void CharacterConfiguration::DisplayAnimationsTab()
{
  _animations.DisplayInGUI();

  if (ImGui::Button("Save Animations"))
    SaveAssetFile("animations.json", _animations);

  ImGui::SameLine();
  if (ImGui::Button("Reload Animation Collection"))
    AnimationCollectionManager::Get().ReloadCharacter(_characterIdentifier);
}

//______________________________________________________________________________
void CharacterConfiguration::DisplayActionsTab()
{
  // --- Action selector ---
  std::vector<std::string> actionNames;
  for (const auto& action : _actions.GetLibrary())
    actionNames.push_back(action.first);

  ImGui::Text("Action:");
  ImGui::SameLine();
  std::string prev = _selectedAction;
  DropDown::DisplayList(actionNames, _selectedAction);
  if (_selectedAction != prev)
    _actionPreview.Reset();

  ImGui::SameLine();
  if (ImGui::Button("Save Actions"))
  {
    _actionPreview.CommitHitbox();
    SaveAssetFile("actions.json", _actions);

    // Rebuild all runtime event lists so changes take effect immediately
    AnimationCollection& collection = GAnimArchive.GetCollection(GAnimArchive.GetCollectionID(_characterIdentifier));
    for (const auto& action : _actions.GetLibrary())
    {
      Animation* spriteAnim = collection.GetSpriteAnimation(action.first);
      if (spriteAnim)
        collection.SetAnimationEvents(action.first, action.second.eventData, action.second.frameData);
    }
  }

  // --- Create / Delete ---
  if (ImGui::CollapsingHeader("Manage Actions"))
  {
    // Build list of animations that don't already have an action
    std::vector<std::string> availableAnims;
    for (const auto& anim : _animations.GetLibrary())
    {
      if (_actions.GetLibrary().find(anim.first) == _actions.GetLibrary().end())
        availableAnims.push_back(anim.first);
    }

    if (availableAnims.empty())
    {
      ImGui::TextDisabled("All animations already have actions.");
    }
    else
    {
      if (_newActionSelection >= static_cast<int>(availableAnims.size()))
        _newActionSelection = 0;

      ImGui::Text("From animation:");
      ImGui::SameLine();
      if (ImGui::BeginCombo("##NewAction", availableAnims[_newActionSelection].c_str()))
      {
        for (int i = 0; i < static_cast<int>(availableAnims.size()); ++i)
        {
          bool selected = (i == _newActionSelection);
          if (ImGui::Selectable(availableAnims[i].c_str(), selected))
            _newActionSelection = i;
        }
        ImGui::EndCombo();
      }
      ImGui::SameLine();
      if (ImGui::Button("Create Action"))
      {
        std::string name = availableAnims[_newActionSelection];
        _actions.GetModifiable(name);
        _selectedAction = name;
        _actionPreview.Reset();
        _newActionSelection = 0;
      }
    }

    if (!_selectedAction.empty())
    {
      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.15f, 0.15f, 1.0f));
      std::string delLabel = "Delete '" + _selectedAction + "'";
      if (ImGui::Button(delLabel.c_str()))
      {
        _actions.Remove(_selectedAction);
        _selectedAction = "";
        _actionPreview.Reset();
      }
      ImGui::PopStyleColor();
    }
  }

  if (_selectedAction.empty())
    return;

  // Verify selection still valid
  if (_actions.GetLibrary().find(_selectedAction) == _actions.GetLibrary().end())
  {
    _selectedAction = "";
    return;
  }

  ActionAsset& data = _actions.GetModifiable(_selectedAction);
  AnimationCollection& collection = GAnimArchive.GetCollection(GAnimArchive.GetCollectionID(_characterIdentifier));
  IAnimation* anim = collection.GetAnimation(_selectedAction);

  ImGui::Separator();

  // --- Frame Data (editable inline) ---
  if (ImGui::CollapsingHeader("Frame Data", ImGuiTreeNodeFlags_DefaultOpen))
  {
    data.frameData.DisplayInEditor();

    ImGui::Spacing();
    if (anim)
    {
      if (ImGui::Button("Apply Frame Data"))
      {
        Animation* spriteAnim = collection.GetSpriteAnimation(_selectedAction);
        if (spriteAnim)
          collection.SetAnimationEvents(_selectedAction, data.eventData, data.frameData);
        _actionPreview.Reset();
      }
      ImGui::SameLine();
      ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "(rebuilds animation timing)");
    }
  }

  if (!anim)
  {
    ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "No animation found for '%s'", _selectedAction.c_str());
    ImGui::TextWrapped("An animation with this name must exist and the collection must be reloaded.");
    return;
  }

  // --- Event Data ---
  if (ImGui::CollapsingHeader("Event Data"))
  {
    int deleteIdx = -1;
    for (int i = 0; i < static_cast<int>(data.eventData.size()); i++)
    {
      ImGui::PushID(i);
      std::string label = "Frame " + std::to_string(i);
      if (ImGui::CollapsingHeader(label.c_str()))
      {
        data.eventData[i].DisplayInEditor();
        if (ImGui::Button("Delete"))
          deleteIdx = i;
      }
      ImGui::PopID();
    }
    if (deleteIdx >= 0)
      data.eventData.erase(data.eventData.begin() + deleteIdx);

    if (ImGui::Button("+ Add Event Frame"))
      data.eventData.emplace_back();
  }

  ImGui::Separator();

  // --- Animation Preview + Hitbox Editing ---
  _actionPreview.Display(anim, data);
}

//______________________________________________________________________________
void ActionPreview::Display(IAnimation* anim, ActionAsset& data)
{
  const FrameData& frameData = data.frameData;
  int totalFrames = anim->GetFrameCount();
  if (totalFrames <= 0)
    return;

  _boundAnim = anim;
  _boundData = &data;

  // Detect frame change — commit old hitbox, load new one
  int prevFrame = _frame;

  // Play/Pause/Step controls
  if (_playing)
  {
    if (ImGui::Button("Pause"))
      _playing = false;
  }
  else
  {
    if (ImGui::Button("Play"))
      _playing = true;
  }
  ImGui::SameLine();
  if (ImGui::Button("|< Reset"))
  {
    _frame = 0;
    _accumTime = 0.0f;
  }
  ImGui::SameLine();
  if (ImGui::Button("< Step") && !_playing)
    _frame = _frame == 0 ? totalFrames - 1 : _frame - 1;
  ImGui::SameLine();
  if (ImGui::Button("Step >") && !_playing)
    _frame = (_frame + 1) % totalFrames;
  ImGui::SameLine();
  ImGui::SliderInt("##PreviewFrame", &_frame, 0, totalFrames - 1);

  // Advance frame based on real time when playing
  if (_playing)
  {
    _accumTime += ImGui::GetIO().DeltaTime;
    while (_accumTime >= secPerFrame)
    {
      _accumTime -= secPerFrame;
      _frame = (_frame + 1) % totalFrames;
    }
  }

  if (_frame >= totalFrames)
    _frame = 0;

  // Hitbox edit toggle
  ImGui::Checkbox("Edit Hitboxes", &_editingHitboxes);
  if (_editingHitboxes)
  {
    ImGui::SameLine();
    if (ImGui::Button("Clear Hitbox"))
      _editRect.ClearGeometry();
  }

  // On frame change: commit the old frame's hitbox, load the new one
  bool frameChanged = (_frame != prevFrame || _lastEvtFrame < 0);
  if (frameChanged)
  {
    if (_lastEvtFrame >= 0 && _editingHitboxes)
      CommitHitboxForFrame(anim, data);
    LoadHitboxForFrame(anim, data);
  }

  // Determine phase based on frame data
  const char* phase = "Raw";
  ImVec4 phaseColor(0.7f, 0.7f, 0.7f, 1.0f);

  bool hasFrameData = (frameData.startUp > 0 && frameData.active > 0 && frameData.recover > 0);
  if (hasFrameData)
  {
    int startupEnd = frameData.startUp - 2;
    int activeEnd = startupEnd + frameData.active;

    if (_frame <= startupEnd)
    {
      phase = "Startup";
      phaseColor = ImVec4(1.0f, 0.9f, 0.2f, 1.0f);
    }
    else if (_frame <= activeEnd)
    {
      phase = "Active";
      phaseColor = ImVec4(1.0f, 0.3f, 0.2f, 1.0f);
    }
    else
    {
      phase = "Recovery";
      phaseColor = ImVec4(0.3f, 0.5f, 1.0f, 1.0f);
    }
  }

  // Frame info line
  ImGui::Text("Frame %d / %d  ", _frame, totalFrames - 1);
  ImGui::SameLine();
  ImGui::TextColored(phaseColor, "[%s]", phase);
  if (hasFrameData)
  {
    ImGui::SameLine();
    ImGui::Text("  S:%d A:%d R:%d", frameData.startUp, frameData.active, frameData.recover);
  }

  // Hitbox status for current frame
  if (_editingHitboxes)
  {
    int evtFrame = anim->GetFrameIndexOffset(_frame);
    if (evtFrame < static_cast<int>(data.eventData.size()) && data.eventData[evtFrame].hitbox.Area() > 0)
      ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "Hitbox set");
    else
      ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No hitbox (left-click to place, right-click to clear)");
  }

  // Phase bar — visual timeline showing startup/active/recovery
  if (hasFrameData && totalFrames > 1)
  {
    float barWidth = ImGui::GetContentRegionAvail().x;
    float barHeight = 12.0f;
    ImVec2 barPos = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    int startupEnd = frameData.startUp - 1;
    int activeEnd = startupEnd + frameData.active;

    float startupW = barWidth * ((float)startupEnd / (float)totalFrames);
    float activeW = barWidth * ((float)frameData.active / (float)totalFrames);
    float recoveryW = barWidth - startupW - activeW;

    dl->AddRectFilled(barPos, ImVec2(barPos.x + startupW, barPos.y + barHeight),
                       IM_COL32(255, 230, 50, 200));
    dl->AddRectFilled(ImVec2(barPos.x + startupW, barPos.y),
                       ImVec2(barPos.x + startupW + activeW, barPos.y + barHeight),
                       IM_COL32(255, 75, 50, 200));
    dl->AddRectFilled(ImVec2(barPos.x + startupW + activeW, barPos.y),
                       ImVec2(barPos.x + startupW + activeW + recoveryW, barPos.y + barHeight),
                       IM_COL32(75, 125, 255, 200));

    // Playhead marker
    float playheadX = barPos.x + barWidth * ((float)_frame / (float)totalFrames);
    dl->AddLine(ImVec2(playheadX, barPos.y - 2), ImVec2(playheadX, barPos.y + barHeight + 2),
                 IM_COL32(255, 255, 255, 255), 2.0f);

    ImGui::Dummy(ImVec2(barWidth, barHeight + 4));
  }

  // Show the frame preview (larger when editing hitboxes)
  int previewHeight = _editingHitboxes ? 512 : 256;
  if (!frameChanged)
  {
    // Reuse cached preview when frame hasn't changed
  }
  else
  {
    int displayFrame = anim->PlaysReverse() ? (totalFrames - 1 - _frame) : _frame;
    _preview = anim->GetEditorPreview(previewHeight, displayFrame);
    _editRect.SetCanvasSize(_preview.displaySize);
  }

  int previewW = std::max(_preview.displaySize.x + 20, 100);
  int previewH = std::max(_preview.displaySize.y + 20, 100);
  ImGui::BeginChild("ActionPreview", ImVec2((float)previewW, (float)previewH), true);
  Vector2<float> imgPos = _preview.Show();
  if (_editingHitboxes)
    _editRect.DisplayAtPosition(imgPos);
  ImGui::EndChild();
}

//______________________________________________________________________________
void CharacterConfiguration::DisplayStatesTab()
{
  std::vector<std::string> animNames;
  for (const auto& a : _animations.GetLibrary())
    animNames.push_back(a.first);

  std::vector<std::string> actNames;
  for (const auto& a : _actions.GetLibrary())
    actNames.push_back(a.first);

  _stateVisualizer.Display(_characterIdentifier, animNames, actNames);
}
