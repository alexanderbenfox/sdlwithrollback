#include "SpriteSheet.h"
#include "Managers/ResourceManager.h"
#include "Managers/GameManagement.h"
#include "../imgui/imgui.h"
#include "AssetLibrary.h"

const ImU32 imColorWhite = IM_COL32(255, 255, 255, 255);

//______________________________________________________________________________
template <> void AssetLoaderFn::OnLoad(SpriteSheet& asset)
{
  ResourceManager::Get().GetAsset<RenderType>(asset.src);
}

//______________________________________________________________________________
template <> ImVec2 AssetLoaderFn::GetDisplaySize<SpriteSheet>()
{
  return ImVec2(700, 8 * fieldHeight);
}

template <> std::string AssetLoaderFn::GUIHeaderLabel<SpriteSheet> = "Sprite Sheets";
template <> std::string AssetLoaderFn::GUIItemLabel<SpriteSheet> = "Sprite Sheet";

//______________________________________________________________________________
std::string SpriteSheet::SaveLocation()
{
  return ResourceManager::Get().GetResourcePath() + "json\\general\\spritesheets.json";
}

static int showFrame = 0;

//______________________________________________________________________________
DrawRect<float> SpriteSheet::Section::GetFrame(int frame) const
{
  if (variableSizeSprites)
  {
    if (frame < (int)frameRects.size())
      return frameRects[frame];
    else
      return DrawRect<float>();
  }
  else
  {
    int x = frame % columns;
    int y = frame / columns;
    Vector2<float> pos(static_cast<float>(offset.x + x * frameSize.x), static_cast<float>(offset.y + y * frameSize.y));
    return DrawRect<float>(pos.x, pos.y, static_cast<float>(frameSize.x), static_cast<float>(frameSize.y));
  }
}

//______________________________________________________________________________
void SpriteSheet::Section::ShowSpriteAtIndex(const SpriteSheet& srcSheet, int frame, int height) const
{
  auto frameRect = GetFrame(frame);
  DisplayImage img((std::string)srcSheet.src, Rect<float>(frameRect.x, frameRect.y, frameRect.x + frameRect.w, frameRect.y + frameRect.h), height);

  // add padding here for some reason
  int borderSizeX = static_cast<int>(static_cast<float>(img.displaySize.x) * 1.25f);
  int borderSizeY = static_cast<int>(static_cast<float>(img.displaySize.y) * 1.25f);

  std::string childLabel = "##SpriteSheetIndex:" + std::to_string(frame);
  ImGui::BeginChild(childLabel.c_str(), ImVec2(borderSizeX, borderSizeY), true);
  Vector2<float> windowPosition = img.Show();
  ImGui::EndChild();
}

//______________________________________________________________________________
void SpriteSheet::Section::ShowSpriteSheetLines(const SpriteSheet& srcSheet)
{
  static float scrollFactor = 1.0f;
  static Vector2<float> renderingOffset = Vector2<float>::Zero;
  static ImVec2 initDragPos;

  const float baseDisplayHeight = 1024;
  int displayHeight = scrollFactor * baseDisplayHeight;

  ImGui::BeginChild("SpriteSheetLines");
  Vector2<int> ss = ResourceManager::Get().GetTextureWidthAndHeight(srcSheet.src);
  DisplayImage img(srcSheet.src, Rect<float>(0, 0, ss.x, ss.y), displayHeight);

  Vector2<float> windowPosition = img.Show(renderingOffset.x, renderingOffset.y);
  windowPosition.x -= ImGui::GetScrollX();
  windowPosition.y -= ImGui::GetScrollY();

  ImVec2 cursor = ImGui::GetMousePos();
  Vector2<float> cursorPos(ImGui::GetMousePos().x - windowPosition.x, ImGui::GetMousePos().y - windowPosition.y);
  // cursor relative to top left of window
  cursorPos += renderingOffset;

  ImVec2 windowSize = ImGui::GetWindowSize();

  // check if its inside window
  if (cursorPos.x > 0 && cursorPos.y > 0 && cursorPos.x < windowSize.x && cursorPos.y < windowSize.y)
  {
    scrollFactor += (ImGui::GetIO().MouseWheel / 5.0f);
    // use right mouse down to check for dragging

    if (ImGui::GetIO().MouseClicked[1])
    {
      initDragPos = cursor;
    }
    else if (ImGui::GetIO().MouseDown[1])
    {
      Vector2<float> delta(initDragPos.x - cursor.x, initDragPos.y - cursor.y);
      initDragPos = cursor;
      renderingOffset -= delta;
    }
  }

  Vector2<float> scaler((float)img.displaySize.x / (float)ss.x, (float)img.displaySize.y / (float)ss.y);
  ImDrawList* draws = ImGui::GetWindowDrawList();

  if (variableSizeSprites)
  {
    if (!frameRects.empty())
    {
      if (showFrame > (int)frameRects.size() || showFrame < 0)
        showFrame = 0;
      DrawRect<int> currFrameRect = frameRects[showFrame];
      currFrameRect.x *= scaler.x;
      currFrameRect.y *= scaler.y;
      currFrameRect.w *= scaler.x;
      currFrameRect.h *= scaler.y;

      ImVec2 rectStart(windowPosition.x + currFrameRect.x, windowPosition.y + currFrameRect.y);
      ImVec2 rectEnd(rectStart.x + currFrameRect.w, rectStart.y + currFrameRect.h);
      draws->AddRect(rectStart, rectEnd, imColorWhite);
    }
  }
  else
  {
    for (int x = 0; x < columns; x++)
    {
      float xPos = offset.x + x * frameSize.x;
      xPos *= scaler.x;

      draws->AddLine(ImVec2(windowPosition.x + xPos, windowPosition.y + scaler.y * offset.y), ImVec2(windowPosition.x + xPos, windowPosition.y + scaler.y * srcSheet.sheetSize.y), imColorWhite);
    }
    for (int y = 0; y < rows; y++)
    {
      float yPos = offset.y + y * frameSize.y;
      yPos *= scaler.y;

      draws->AddLine(ImVec2(windowPosition.x + scaler.x * offset.x, windowPosition.y + yPos), ImVec2(windowPosition.x + scaler.x * srcSheet.sheetSize.x, windowPosition.y + yPos), imColorWhite);
    }
  }

  ImGui::EndChild();

  ImGui::Begin("SpriteEditorSidePanel");
  if (ImGui::Button("Switch to Frame View"))
  {
    GUIController::Get().CreatePopup("Sprite Sheet Editor", [this, &srcSheet]() { DisplaySpriteEditor(srcSheet, showFrame); }, []() {});
  }

  ImGui::InputInt("Show Frame", &showFrame);
  if (variableSizeSprites)
  {
    if (!frameRects.empty())
    {
      if (showFrame >= (int)frameRects.size())
        showFrame = (int)frameRects.size() - 1;
      else if (showFrame < 0)
        showFrame = 0;
      DrawRect<int>& currFrameRect = frameRects[showFrame];
      ImGui::DragInt2("Top Left Position", &currFrameRect.x, 1.0f, -4096, 8192);
      ImGui::DragInt2("Size", &currFrameRect.w);
    }
    if (ImGui::Button("Add New Empty Frame")) { frameRects.emplace_back(); }
    if (!frameRects.empty())
    {
      auto CopyFrameRect = [this]()
      {
        DrawRect<int> toCopy = frameRects[showFrame];
        frameRects.emplace_back(toCopy);
        showFrame = frameRects.size() - 1;
        return Vector2<int>(toCopy.w, toCopy.h);
      };

      ImGui::SameLine();
      if (ImGui::Button("Copy Shown Frame")) { CopyFrameRect(); }

      ImGui::Text("Copy Shown Frame with Offset");
      ImGui::SameLine();
      if (ImGui::ArrowButton("##sheetArrow_Left", ImGuiDir_Left))
      {
        auto size = CopyFrameRect();
        frameRects.back().x -= size.x;
      }
      ImGui::SameLine();
      if (ImGui::ArrowButton("##sheetArrow_Right", ImGuiDir_Right))
      {
        auto size = CopyFrameRect();
        frameRects.back().x += size.x;
      }
      ImGui::SameLine();
      if (ImGui::ArrowButton("##sheetArrow_Up", ImGuiDir_Up))
      {
        auto size = CopyFrameRect();
        frameRects.back().y -= size.y;
      }
      ImGui::SameLine();
      if (ImGui::ArrowButton("##sheetArrow_Down", ImGuiDir_Down))
      {
        auto size = CopyFrameRect();
        frameRects.back().y += size.y;
      }
    }
    ShowVariableFrames(true);
  }
  else
  {
    ImGui::DragInt2("Sheet Start Offset", &offset.x, 1.0f, -4096, 8192);
    ImGui::DragInt2("Frame Size", &frameSize.x);
  }

  if (ImGui::Button("Reset View"))
  {
    scrollFactor = 1.0f;
    renderingOffset = Vector2<float>::Zero;
    initDragPos.x = 0;
    initDragPos.y = 0;
  }
  ImGui::End();
}

//______________________________________________________________________________
void SpriteSheet::Section::DisplaySpriteEditor(const SpriteSheet& srcSheet, int frame)
{
  if (variableSizeSprites)
  {
    if (!frameRects.empty())
    {
      if (showFrame >= (int)frameRects.size())
        showFrame = (int)frameRects.size() - 1;
      else if (showFrame < 0)
        showFrame = 0;

      DisplaySpriteEditorInternal(srcSheet, showFrame);
    }
  }
  else
    DisplaySpriteEditorInternal(srcSheet, showFrame);
}

//______________________________________________________________________________
void SpriteSheet::Section::LoadJson(const Json::Value& json)
{
  if (!json["variableSizeSprites"].isNull())
  {
    variableSizeSprites = json["variableSizeSprites"].asBool();
  }
  if (variableSizeSprites)
  {
    // clear frame rects before loading
    frameRects.clear();
    auto& frameRectsJson = json["frameRects"];
    if (!frameRectsJson.isNull() && frameRectsJson.isArray())
    {
      frameRects.resize(frameRectsJson.size());
      for (Json::Value::ArrayIndex i = 0; i != frameRectsJson.size(); i++)
      {
        frameRects[i].x = frameRectsJson[i]["x"].asInt();
        frameRects[i].y = frameRectsJson[i]["y"].asInt();
        frameRects[i].w = frameRectsJson[i]["w"].asInt();
        frameRects[i].h = frameRectsJson[i]["h"].asInt();
      }
    }

    // set these for the sprite sheet viewer?
    rows = 1;
columns = 1;
  }
  else
  {
  if (!json["rows"].isNull())
  {
    rows = json["rows"].asInt();
  }
  if (!json["columns"].isNull())
  {
    columns = json["columns"].asInt();
  }
  if (!json["frameSize"].isNull())
  {
    frameSize = Vector2<int>(json["frameSize"]["x"].asInt(), json["frameSize"]["y"].asInt());
  }
  if (!json["offset"].isNull())
  {
    offset = Vector2<int>(json["offset"]["x"].asInt(), json["offset"]["y"].asInt());
  }
  }
}

//______________________________________________________________________________
void SpriteSheet::Section::WriteJson(Json::Value& json) const
{
  json["variableSizeSprites"] = variableSizeSprites;
  if (variableSizeSprites)
  {
    // set as array value
    json["frameRects"] = Json::Value(Json::ValueType::arrayValue);
    for (int i = 0; i < frameRects.size(); i++)
    {
      Json::Value rectValue;
      rectValue["x"] = frameRects[i].x;
      rectValue["y"] = frameRects[i].y;
      rectValue["w"] = frameRects[i].w;
      rectValue["h"] = frameRects[i].h;
      json["frameRects"].append(rectValue);
    }
  }
  else
  {
    json["rows"] = rows;
    json["columns"] = columns;
    json["frameSize"]["x"] = frameSize.x;
    json["frameSize"]["y"] = frameSize.y;
    json["offset"]["x"] = offset.x;
    json["offset"]["y"] = offset.y;
  }
}

//______________________________________________________________________________
void SpriteSheet::Section::DisplayInEditor(SpriteSheet& srcSheet)
{
  if (ImGui::Checkbox("Variable Size Sprites", &variableSizeSprites))
  {
    if (variableSizeSprites)
    {
      columns = 1;
      rows = 1;
    }
  }

  if (!variableSizeSprites)
  {
    ImGui::DragInt2("Sheet Start Offset", &offset.x, 1.0f, -4096, 8192);
    ImGui::DragInt2("Frame Size", &frameSize.x);
    ImGui::InputInt2("Rows & Columns: ", &rows);
    if (ImGui::Button("Generate Frame Size Based On Columns & Rows"))
    {
      frameSize = Vector2<int>(srcSheet.sheetSize.x / columns, srcSheet.sheetSize.y / rows);
    }
  }
  else
  {
    ShowVariableFrames(false);
    if(ImGui::Button("Add New Empty Frame")) { frameRects.emplace_back(); }
    if (!frameRects.empty())
    {
      ImGui::SameLine();
      if (ImGui::Button("Copy Last Frame")) { frameRects.emplace_back(frameRects.back()); }
    }
  }

  if (ImGui::Button("Show"))
  {
    GUIController::Get().CreatePopup("Sprite Sheet Editor", [this, &srcSheet]() { DisplaySpriteEditor(srcSheet, showFrame); }, []() {});
  }
  ImGui::SameLine();
  ImGui::PushItemWidth(70.0f);
  ImGui::InputInt("Show Frame", &showFrame);
  ImGui::PopItemWidth();
  ImGui::SameLine();
  if (ImGui::Button("Show Sheet Lines"))
  {
    GUIController::Get().CreatePopup("Sprite Sheet Editor", [this, &srcSheet]() { ShowSpriteSheetLines(srcSheet); }, []() {});
  }
}

//______________________________________________________________________________
void SpriteSheet::Section::ShowVariableFrames(bool switchShown)
{
  if (ImGui::CollapsingHeader("Frame Sizes"))
  {
    int deleteIdx = -1;
    std::tuple<bool, int, int> swapOperation = std::make_tuple(false, 0, 0);
    for (int i = 0; i < frameRects.size(); i++)
    {
      auto frameStr = std::to_string(i);
      std::string arrowIDUp = "##arrowIDUp" + frameStr;
      std::string arrowIDDown = "##arrowIDDown" + frameStr;

      ImGui::Dummy(ImVec2(10.0f, 0.0f));
      ImGui::SameLine();
      std::string rmFrameBtn = "Remove " + std::to_string(i);
      if (ImGui::Button(rmFrameBtn.c_str()))
      {
        deleteIdx = i;
      }

      if (i != 0)
      {
        ImGui::SameLine();
        if (ImGui::ArrowButton(arrowIDUp.c_str(), ImGuiDir_Up))
        {
          swapOperation = { true, i, i - 1 };
        }
      }

      if (i != (frameRects.size() - 1))
      {
        ImGui::SameLine();
        if (ImGui::ArrowButton(arrowIDDown.c_str(), ImGuiDir_Down))
        {
          swapOperation = { true, i, i + 1 };
        }
      }
      ImGui::SameLine();
      if (ImGui::CollapsingHeader(frameStr.c_str()))
      {
        auto& currFrameRect = frameRects[i];
        ImGui::Dummy(ImVec2(50.0f, 0.0f));
        ImGui::SameLine();
        ImGui::DragInt2("Top Left Position", &currFrameRect.x, 1.0f, -4096, 8192);
        ImGui::Dummy(ImVec2(50.0f, 0.0f));
        ImGui::SameLine();
        ImGui::DragInt2("Size", &currFrameRect.w);
      }
    }
    if (deleteIdx >= 0)
      frameRects.erase(frameRects.begin() + deleteIdx);
    if (std::get<0>(swapOperation))
    {
      int a = std::get<1>(swapOperation);
      int b = std::get<2>(swapOperation);

      auto cpy = frameRects[a];
      frameRects[a] = frameRects[b];
      frameRects[b] = cpy;
      if (switchShown)
        showFrame = b;
    }
  }
}

//______________________________________________________________________________
void SpriteSheet::Section::DisplaySpriteEditorInternal(const SpriteSheet& srcSheet, int frame)
{
  ImGui::BeginChild("FrameDisplay");
  ImGui::InputInt("Show Frame", &showFrame);
  if (variableSizeSprites)
  {
    DrawRect<int>& currFrameRect = frameRects[frame];
    ImGui::DragInt2("Top Left Position", &currFrameRect.x, 0.5f, -4096, 8192);
    ImGui::DragInt2("Size", &currFrameRect.w);
  }
  else
  {
    ImGui::DragInt2("Sheet Start Offset", &offset.x, 0.5f, -4096, 8192);
    ImGui::DragInt2("Frame Size", &frameSize.x);
  }

  ShowSpriteAtIndex(srcSheet, frame, 512);

  if (ImGui::Button("Switch to Sheet View"))
  {
    GUIController::Get().CreatePopup("Sprite Sheet Editor", [this, &srcSheet]() { ShowSpriteSheetLines(srcSheet); }, []() {});
  }
  ImGui::EndChild();
}

//______________________________________________________________________________
void SpriteSheet::GenerateSheetInfo()
{
  sheetSize = ResourceManager::Get().GetTextureWidthAndHeight(src);
  if(mainSection.columns > 0 && mainSection.rows > 0)
    mainSection.frameSize = Vector2<int>(sheetSize.x / mainSection.columns, sheetSize.y / mainSection.rows);
}

//______________________________________________________________________________
void SpriteSheet::Load(const Json::Value& json)
{
  if (!json["sheet_location"].isNull())
  {
    src = json["sheet_location"].asString();
  }
  if (!json["sheetSize"].isNull())
  {
    sheetSize = Vector2<int>(json["sheetSize"]["x"].asInt(), json["sheetSize"]["y"].asInt());
  }
  if (!json["renderScalingFactor"].isNull())
  {
    renderScalingFactor = Vector2<double>(json["renderScalingFactor"]["x"].asDouble(), json["renderScalingFactor"]["y"].asDouble());
  }

  // load main section if it exists
  mainSection.LoadJson(json);

  if (!json["SubSections"].isNull())
  {
    for (const auto& section : json["SubSections"].getMemberNames())
    {
      subSections[section].LoadJson(json["SubSections"][section]);
    }
  }

  if (sheetSize.x == 0 || sheetSize.y == 0)
    GenerateSheetInfo();
}

//______________________________________________________________________________
void SpriteSheet::Write(Json::Value& json) const
{
  json["sheet_location"] = (std::string)src;
  json["sheetSize"]["x"] = sheetSize.x;
  json["sheetSize"]["y"] = sheetSize.y;
  json["renderScalingFactor"]["x"] = renderScalingFactor.x;
  json["renderScalingFactor"]["y"] = renderScalingFactor.y;

  // write out the sections
  mainSection.WriteJson(json);
  for (const auto& section : subSections)
  {
    section.second.WriteJson(json["SubSections"][section.first]);
  }
}

//______________________________________________________________________________
void SpriteSheet::DisplayInEditor()
{
  src.DisplayEditable("File Path (Relative To Resource Folder)");

  {
    ImGui::InputInt2("Sheet Size", &sheetSize.x);
    ImGui::SameLine();
    if (ImGui::Button("Generate")) { GenerateSheetInfo(); }
  }

  ImGui::InputDouble("Render Scaling Factor X", &renderScalingFactor.x);
  ImGui::InputDouble("Render Scaling Factor Y", &renderScalingFactor.y);

  if (ImGui::CollapsingHeader("Main Sprite Sheet Segment"))
  {
    mainSection.DisplayInEditor(*this);
    if (ImGui::Button("Add As Subsection"))
    {
      static EditorString subsectionString;
      GUIController::Get().CreatePopup("Sprite Sheet Subsection",
        []()
        {
          subsectionString.DisplayEditable("Subsection Name");
        },
        [this]()
        {
          subSections[(std::string)subsectionString] = mainSection;
          subsectionString.clear();
        }, 350, 4 * fieldHeight);
    }
  }
    

  if (!subSections.empty())
  {
    if (ImGui::CollapsingHeader("Sub Sections"))
    {
      std::pair<bool, std::string> deleteCall = { false, "" };
      for (auto& section : subSections)
      {
        ImGui::Dummy(ImVec2(20.0f, 0.0f));
        ImGui::SameLine();
        if (ImGui::Button("Erase Section")) { deleteCall = { true, section.first }; break; }
        ImGui::SameLine();
        if(ImGui::CollapsingHeader(section.first.c_str()))
          section.second.DisplayInEditor(*this);
      }
      if (deleteCall.first)
        subSections.erase(deleteCall.second);
    }
  }
}
