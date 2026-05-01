#include "AnimationAsset.h"
#include "DebugGUI/GUIController.h"
#include "DebugGUI/DisplayImage.h"

#include "AssetManagement/Animation.h"
#include "AssetLibraryImpl.h"

// this is for the find anchor
#include "Managers/GameManagement.h"

//______________________________________________________________________________
namespace std
{
  string to_string(AnchorPoint value)
  {
    switch (value)
    {
    case AnchorPoint::BL: return "BL";
    case AnchorPoint::TL: return "TL";
    case AnchorPoint::BR: return "BR";
    case AnchorPoint::TR: return "TL";
    case AnchorPoint::Center: return "Center";
    default: return "NaN";
    }
  }
}

//______________________________________________________________________________
AnchorPoint APFromString(const std::string i)
{
  if (i == "TL")
    return AnchorPoint::TL;
  else if (i == "TR")
    return AnchorPoint::TR;
  else if (i == "BL")
    return AnchorPoint::BL;
  else if (i == "BR")
    return AnchorPoint::BR;
  else
    return AnchorPoint::Center;
}

//______________________________________________________________________________
Vector2<float> CalculateRenderOffset(AnchorPoint anchor, const Vector2<float>& textureRenderOffset, const Vector2<float>& rectTransform)
{
  Vector2<float> offset = textureRenderOffset;

  if (anchor == AnchorPoint::TL) {}
  else if (anchor == AnchorPoint::BL)
  {
    offset.y -= rectTransform.y;
  }
  else if (anchor == AnchorPoint::TR)
  {
    offset.x -= rectTransform.x;
  }
  else if (anchor == AnchorPoint::BR)
  {
    offset -= rectTransform;
  }
  else if (anchor == AnchorPoint::Center)
  {
    offset -= (rectTransform / 2.0f);
  }

  return -rectTransform / 2.0f - offset;
}

//______________________________________________________________________________
template <> void AssetLoaderFn::OnLoad(AnimationAsset& asset) {}

//______________________________________________________________________________
template <> ImVec2 AssetLoaderFn::GetDisplaySize<AnimationAsset>()
{
  return ImVec2(500, 10 * fieldHeight);
}

//______________________________________________________________________________
void AnimationAsset::Load(const Json::Value& json)
{
  if (!json["sheet_name"].isNull())
  {
    sheetName = json["sheet_name"].asString();
  }
  if (!json["sub_sheet_name"].isNull())
  {
    subSheetName = json["sub_sheet_name"].asString();
  }
  if (!json["startFrame"].isNull())
  {
    startIndexOnSheet = json["startFrame"].asInt();
  }
  if (!json["totalFrames"].isNull())
  {
    frames = json["totalFrames"].asInt();
  }
  if (!json["anchor"].isNull())
  {
    anchor = APFromString(json["anchor"].asString());
  }
  if (!json["anchorPoints"].isNull())
  {
    auto ap = json["anchorPoints"];
    anchorPoints[(int)AnchorPoint::TL].Load(ap["TL"]);
    anchorPoints[(int)AnchorPoint::TR].Load(ap["TR"]);
    anchorPoints[(int)AnchorPoint::BL].Load(ap["BL"]);
    anchorPoints[(int)AnchorPoint::BR].Load(ap["BR"]);
    anchorPoints[(int)AnchorPoint::Center].Load(ap["Center"]);
  }
  if (!json["reverse"].isNull())
  {
    reverse = json["reverse"].asBool();
  }
  if (json.isMember("hurtboxes") && json["hurtboxes"].isArray())
  {
    for (const auto& hb : json["hurtboxes"])
    {
      Vector2<double> pos(hb["position"]["x"].asDouble(), hb["position"]["y"].asDouble());
      Vector2<double> sz(hb["size"]["x"].asDouble(), hb["size"]["y"].asDouble());
      hurtboxes.emplace_back(pos.x, pos.y, pos.x + sz.x, pos.y + sz.y);
    }
  }
}

//______________________________________________________________________________
void AnimationAsset::Write(Json::Value& json) const
{
  json["sheet_name"] = (std::string)sheetName;
  json["sub_sheet_name"] = (std::string)subSheetName;
  json["startFrame"] = startIndexOnSheet;
  json["totalFrames"] = frames;
  json["anchor"] = std::to_string(anchor);

  auto& ap = json["anchorPoints"] = Json::Value(Json::objectValue);

  anchorPoints[(int)AnchorPoint::TL].Write(ap["TL"]);
  anchorPoints[(int)AnchorPoint::TR].Write(ap["TR"]);
  anchorPoints[(int)AnchorPoint::BL].Write(ap["BL"]);
  anchorPoints[(int)AnchorPoint::BR].Write(ap["BR"]);
  anchorPoints[(int)AnchorPoint::Center].Write(ap["Center"]);

  json["reverse"] = reverse;

  if (!hurtboxes.empty())
  {
    Json::Value& hbArray = json["hurtboxes"] = Json::Value(Json::arrayValue);
    for (const auto& hb : hurtboxes)
    {
      Json::Value entry(Json::objectValue);
      entry["position"]["x"] = hb.beg.x;
      entry["position"]["y"] = hb.beg.y;
      entry["size"]["x"] = hb.Width();
      entry["size"]["y"] = hb.Height();
      hbArray.append(entry);
    }
  }
}

//______________________________________________________________________________
void AnimationAsset::DisplayInEditor()
{
  ImGui::BeginGroup();
  sheetName.DisplayEditable("Name of SpriteSheet");
  subSheetName.DisplayEditable("Name of Sub Sheet");
  ImGui::InputInt("Start Index ", &startIndexOnSheet);
  ImGui::InputInt("Total Animation Frames: ", &frames);

  ImGui::Text("Anchor Position");
  const char* items[] = { "TL", "TR", "BL", "BR", "Center" };
  _anchorDropDownCurrentItem = std::to_string(anchor);
  auto func = [this](const std::string& i) { anchor = APFromString(i); };
  DropDown::Show(_anchorDropDownCurrentItem.c_str(), items, 5, func);

  DisplayAnchorPointEditor();

  ImGui::Checkbox("Play Reverse", &reverse);

  // Animation preview — only renders if visible on screen
  if (frames > 0 && ImGui::IsRectVisible(ImVec2(80, 80)))
  {
    const SpriteSheet& animSpriteSheet = ResourceManager::Get().gSpriteSheets.Get(sheetName);
    const SpriteSheet::Section& ssSection = animSpriteSheet.GetSubSection(subSheetName);

    // Advance playback when playing
    if (_playing)
    {
      constexpr float kFrameInterval = 1.0f / 10.0f; // 10 fps preview
      _playbackAccumulator += ImGui::GetIO().DeltaTime;
      while (_playbackAccumulator >= kFrameInterval)
      {
        _playbackAccumulator -= kFrameInterval;
        _previewFrame++;
        if (_previewFrame >= frames)
        {
          if (_looping)
            _previewFrame = 0;
          else
          {
            _previewFrame = frames - 1;
            _playing = false;
          }
        }
      }
    }

    // Clamp frame to valid range
    if (_previewFrame >= frames) _previewFrame = 0;

    if (!_editingHurtboxes)
    {
      // Small preview when not editing
      int sheetIdx = reverse ? (startIndexOnSheet + frames - 1 - _previewFrame)
                             : (startIndexOnSheet + _previewFrame);
      ssSection.ShowSpriteAtIndex(animSpriteSheet, sheetIdx, 64);
    }

    // Playback controls
    ImGui::Text("Frame %d / %d", _previewFrame + 1, frames);
    ImGui::SameLine();
    if (ImGui::Button(_playing ? "Stop" : "Play"))
    {
      if (_playing)
        _playing = false;
      else
      {
        _playing = true;
        _previewFrame = 0;
        _playbackAccumulator = 0.0f;
      }
    }
    ImGui::SameLine();
    ImGui::Checkbox("Loop", &_looping);

    // Manual frame stepping (only when not playing)
    if (!_playing && frames > 1)
    {
      ImGui::SameLine();
      if (ImGui::ArrowButton("##prev", ImGuiDir_Left))
        _previewFrame = (_previewFrame == 0) ? frames - 1 : _previewFrame - 1;
      ImGui::SameLine();
      if (ImGui::ArrowButton("##next", ImGuiDir_Right))
        _previewFrame = (_previewFrame + 1) % frames;
    }

    // Hurtbox editing section
    ImGui::Checkbox("Edit Hurtboxes", &_editingHurtboxes);
    if (_editingHurtboxes)
    {
      ImGui::SameLine();
      if (ImGui::Button("Clear Hurtbox"))
        _hurtboxEditRect.ClearGeometry();

      ImGui::SameLine();
      if (ImGui::Button("Copy to All Frames"))
      {
        CommitHurtboxForFrame();
        if (_hurtboxLastFrame >= 0 && _hurtboxLastFrame < static_cast<int>(hurtboxes.size()))
        {
          Rect<double> current = hurtboxes[_hurtboxLastFrame];
          hurtboxes.resize(frames);
          for (auto& hb : hurtboxes)
            hb = current;
        }
      }

      // Detect frame change — commit old data, load new
      bool frameChanged = (_previewFrame != _hurtboxLastFrame);
      if (frameChanged)
      {
        CommitHurtboxForFrame();
        LoadHurtboxForFrame(_previewFrame);
      }

      // Hurtbox status
      if (_previewFrame < static_cast<int>(hurtboxes.size()) && hurtboxes[_previewFrame].Area() > 0)
        ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "Hurtbox set");
      else
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No hurtbox (left-click to place, right-click to clear)");

      // Large preview with hurtbox overlay
      int sheetIdx = reverse ? (startIndexOnSheet + frames - 1 - _previewFrame)
                             : (startIndexOnSheet + _previewFrame);
      DrawRect<float> frameRect = ssSection.GetFrame(sheetIdx);
      DisplayImage preview(animSpriteSheet.src,
        Rect<float>(frameRect.x, frameRect.y, frameRect.x + frameRect.w, frameRect.y + frameRect.h), 512);

      _hurtboxEditRect.SetCanvasSize(preview.displaySize);

      int previewW = std::max(preview.displaySize.x + 20, 100);
      int previewH = std::max(preview.displaySize.y + 20, 100);
      ImGui::BeginChild("HurtboxPreview", ImVec2((float)previewW, (float)previewH), true);
      Vector2<float> imgPos = preview.Show();
      _hurtboxEditRect.DisplayAtPosition(imgPos);
      ImGui::EndChild();
    }
    else
    {
      // Reset hurtbox editor state when toggled off
      if (_hurtboxLastFrame >= 0)
      {
        CommitHurtboxForFrame();
        _hurtboxLastFrame = -1;
      }
    }
  }

  ImGui::EndGroup();
}

//______________________________________________________________________________
void AnimationAsset::DisplayAnchorPointEditor()
{
  if (_editorWindowDisplayed)
  {
    ImGui::Begin("Anchor Point Editor");
    if (!_anchorEditBackgroundInit)
    {
      const SpriteSheet& animSpriteSheet = ResourceManager::Get().gSpriteSheets.Get(sheetName);
      const SpriteSheet::Section& ssSection = animSpriteSheet.GetSubSection(subSheetName);
      DrawRect<float> frameRect = ssSection.GetFrame(startIndexOnSheet);
      _anchorEditBackground = DisplayImage(animSpriteSheet.src, Rect<float>(frameRect.x, frameRect.y, frameRect.x + frameRect.w, frameRect.y + frameRect.h), 512);

      if (_anchorEditBackground.ptr)
      {
        for (int i = 0; i < (int)AnchorPoint::Size; i++)
        {
          anchorPoints[(int)i].SetCanvasSize(_anchorEditBackground.displaySize);
        }
        _anchorEditBackgroundInit = true;
      }
    }

    if (_anchorEditBackground.ptr)
    {
      std::string anchorPtLabel = std::to_string(anchor);
      ImGui::Text("Displaying Anchor Point = %s", anchorPtLabel.c_str());

      // display image within child
      ImGui::BeginChild("Anchor Point Editor");

      Vector2<float> position = _anchorEditBackground.Show();
      anchorPoints[(int)anchor].DisplayAtPosition(position);

      ImGui::EndChild();
    }

    if (ImGui::Button("Close"))
    {
      _editorWindowDisplayed = false;
    }
    ImGui::End();
  }
  else
  {
    if (ImGui::Button("Edit Anchor Points"))
    {
      _editorWindowDisplayed = true;
      _anchorEditBackgroundInit = false;
    }
  }

}

//______________________________________________________________________________
Vector2<float> AnimationAsset::GetAnchorPosition(int animationFrame) const
{
  const SpriteSheet& animSpriteSheet = ResourceManager::Get().gSpriteSheets.Get(sheetName);
  const SpriteSheet::Section& ssSection = animSpriteSheet.GetSubSection(subSheetName);
  DrawRect<float> rect = ssSection.GetFrame(startIndexOnSheet + animationFrame);

  Vector2<double> const& anchPos = anchorPoints[(int)anchor].Export(Vector2<double>(rect.w, rect.h));
  return static_cast<Vector2<float>>(anchPos);
}

//______________________________________________________________________________
void AnimationAsset::LoadHurtboxForFrame(int sheetFrame)
{
  const SpriteSheet& animSpriteSheet = ResourceManager::Get().gSpriteSheets.Get(sheetName);
  const SpriteSheet::Section& ssSection = animSpriteSheet.GetSubSection(subSheetName);
  DrawRect<float> frameRect = ssSection.GetFrame(startIndexOnSheet + sheetFrame);

  _hurtboxSrcSize = Vector2<double>(frameRect.w, frameRect.h);
  _hurtboxLastFrame = sheetFrame;

  // Import existing hurtbox data if available
  if (sheetFrame < static_cast<int>(hurtboxes.size()) && hurtboxes[sheetFrame].Area() > 0)
    _hurtboxEditRect.Import(hurtboxes[sheetFrame], _hurtboxSrcSize);
  else
    _hurtboxEditRect.ClearGeometry();
}

//______________________________________________________________________________
void AnimationAsset::CommitHurtboxForFrame()
{
  if (_hurtboxLastFrame < 0)
    return;

  // Ensure vector is large enough
  if (_hurtboxLastFrame >= static_cast<int>(hurtboxes.size()))
    hurtboxes.resize(_hurtboxLastFrame + 1);

  if (_hurtboxEditRect.UserDataExists())
    hurtboxes[_hurtboxLastFrame] = _hurtboxEditRect.Export(_hurtboxSrcSize);
  else
    hurtboxes[_hurtboxLastFrame] = Rect<double>();
}

