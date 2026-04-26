#include "AssetManagement/EditableAssets/Editor/SpriteDetector.h"
#include "Managers/ResourceManager.h"
#include "DebugGUI/DisplayImage.h"

#include <stack>

//______________________________________________________________________________
bool SpriteDetector::IsForeground(const Uint8* pixels, int x, int y,
                                   int bpp, int pitch,
                                   SDL_Color bg, int threshold) const
{
  const Uint8* p = pixels + y * pitch + x * bpp;
  int dr = std::abs((int)p[0] - (int)bg.r);
  int dg = std::abs((int)p[1] - (int)bg.g);
  int db = std::abs((int)p[2] - (int)bg.b);

  // If image has alpha channel, fully transparent pixels are background
  if (bpp >= 4 && p[3] < 10)
    return false;

  return (dr > threshold || dg > threshold || db > threshold);
}

//______________________________________________________________________________
SpriteDetector::Result SpriteDetector::FloodFill(
    const Uint8* pixels, int w, int h, int bpp, int pitch,
    int startX, int startY,
    std::vector<bool>& visited,
    SDL_Color bg, int threshold) const
{
  Result result;
  result.bounds = DrawRect<int>(startX, startY, 1, 1);
  result.pixelCount = 0;

  int minX = startX, maxX = startX;
  int minY = startY, maxY = startY;

  std::stack<std::pair<int, int>> stack;
  stack.push({startX, startY});
  visited[startY * w + startX] = true;

  while (!stack.empty())
  {
    auto [cx, cy] = stack.top();
    stack.pop();

    result.pixelCount++;
    minX = std::min(minX, cx);
    maxX = std::max(maxX, cx);
    minY = std::min(minY, cy);
    maxY = std::max(maxY, cy);

    // 4-connected neighbors
    const int dx[] = {-1, 1, 0, 0};
    const int dy[] = {0, 0, -1, 1};
    for (int d = 0; d < 4; d++)
    {
      int nx = cx + dx[d];
      int ny = cy + dy[d];
      if (nx >= 0 && nx < w && ny >= 0 && ny < h)
      {
        int idx = ny * w + nx;
        if (!visited[idx] && IsForeground(pixels, nx, ny, bpp, pitch, bg, threshold))
        {
          visited[idx] = true;
          stack.push({nx, ny});
        }
      }
    }
  }

  result.bounds = DrawRect<int>(minX, minY, maxX - minX + 1, maxY - minY + 1);
  return result;
}

//______________________________________________________________________________
void SpriteDetector::MergeNearby(std::vector<Result>& rects, int gap) const
{
  // Iteratively merge until no more merges happen
  bool merged = true;
  while (merged)
  {
    merged = false;
    for (size_t i = 0; i < rects.size(); i++)
    {
      for (size_t j = i + 1; j < rects.size(); j++)
      {
        auto& a = rects[i].bounds;
        auto& b = rects[j].bounds;

        // Check if bounding boxes overlap or are within 'gap' pixels
        bool overlaps =
          a.x - gap <= b.x + b.w &&
          a.x + a.w + gap >= b.x &&
          a.y - gap <= b.y + b.h &&
          a.y + a.h + gap >= b.y;

        if (overlaps)
        {
          int newX = std::min(a.x, b.x);
          int newY = std::min(a.y, b.y);
          int newR = std::max(a.x + a.w, b.x + b.w);
          int newB = std::max(a.y + a.h, b.y + b.h);
          a = DrawRect<int>(newX, newY, newR - newX, newB - newY);
          rects[i].pixelCount += rects[j].pixelCount;
          rects.erase(rects.begin() + j);
          merged = true;
          break;
        }
      }
      if (merged) break;
    }
  }
}

//______________________________________________________________________________
void SpriteDetector::SortReadingOrder(std::vector<Result>& rects) const
{
  // Group sprites into rows: sprites whose vertical centers are within
  // a tolerance are considered the same row.
  // Sort by Y center first, then group, then sort each group by X.

  if (rects.empty()) return;

  // Sort by vertical center
  std::sort(rects.begin(), rects.end(), [](const Result& a, const Result& b)
  {
    return (a.bounds.y + a.bounds.h / 2) < (b.bounds.y + b.bounds.h / 2);
  });

  // Group into rows: sprites are in the same row if their Y ranges overlap
  std::vector<std::vector<Result>> rows;
  rows.push_back({rects[0]});

  for (size_t i = 1; i < rects.size(); i++)
  {
    auto& lastRow = rows.back();
    int rowMinY = lastRow.front().bounds.y;
    int rowMaxY = 0;
    for (auto& r : lastRow)
      rowMaxY = std::max(rowMaxY, r.bounds.y + r.bounds.h);

    int cy = rects[i].bounds.y + rects[i].bounds.h / 2;
    if (cy >= rowMinY && cy <= rowMaxY)
      lastRow.push_back(rects[i]);
    else
      rows.push_back({rects[i]});
  }

  // Sort each row by X position
  rects.clear();
  for (auto& row : rows)
  {
    std::sort(row.begin(), row.end(), [](const Result& a, const Result& b)
    {
      return a.bounds.x < b.bounds.x;
    });
    for (auto& r : row)
      rects.push_back(r);
  }
}

//______________________________________________________________________________
std::vector<SpriteDetector::Result> SpriteDetector::Detect(
    const std::string& imagePath,
    SDL_Color bg, int threshold, int minAreaParam, int mergeGapParam)
{
  std::vector<Result> detected;

  SDL_Surface* raw = IMG_Load(imagePath.c_str());
  if (!raw)
    return detected;

  // Convert to a consistent pixel format (RGBA32) for uniform access
  SDL_Surface* surface = SDL_ConvertSurfaceFormat(raw, SDL_PIXELFORMAT_RGBA32, 0);
  SDL_FreeSurface(raw);
  if (!surface)
    return detected;

  int w = surface->w;
  int h = surface->h;
  int bpp = surface->format->BytesPerPixel;
  int pitch = surface->pitch;
  const Uint8* pixels = static_cast<const Uint8*>(surface->pixels);

  std::vector<bool> visited(w * h, false);

  // Scan every pixel, flood-fill when hitting an unvisited foreground pixel
  for (int y = 0; y < h; y++)
  {
    for (int x = 0; x < w; x++)
    {
      int idx = y * w + x;
      if (!visited[idx] && IsForeground(pixels, x, y, bpp, pitch, bg, threshold))
      {
        Result r = FloodFill(pixels, w, h, bpp, pitch, x, y, visited, bg, threshold);
        detected.push_back(r);
      }
    }
  }

  SDL_FreeSurface(surface);

  // Merge nearby components (detached sprite parts, e.g. weapon + character)
  if (mergeGapParam > 0)
    MergeNearby(detected, mergeGapParam);

  // Filter by minimum bounding-box area
  detected.erase(
    std::remove_if(detected.begin(), detected.end(),
      [minAreaParam](const Result& r) { return r.bounds.w * r.bounds.h < minAreaParam; }),
    detected.end());

  SortReadingOrder(detected);
  return detected;
}

//______________________________________________________________________________
bool SpriteDetector::DisplayDetectionUI(const std::string& sheetPath,
                                         std::vector<DrawRect<int>>& outFrameRects)
{
  bool applied = false;

  // --- Configuration controls ---
  int bgArr[3] = {bgColor.r, bgColor.g, bgColor.b};
  ImGui::InputInt3("BG Color (R,G,B)", bgArr);
  bgColor.r = static_cast<Uint8>(std::clamp(bgArr[0], 0, 255));
  bgColor.g = static_cast<Uint8>(std::clamp(bgArr[1], 0, 255));
  bgColor.b = static_cast<Uint8>(std::clamp(bgArr[2], 0, 255));

  ImGui::SliderInt("Color Threshold", &colorThreshold, 1, 100);
  ImGui::SliderInt("Min Area (px^2)", &minArea, 10, 10000);
  ImGui::SliderInt("Merge Gap (px)", &mergeGap, 0, 50);

  if (ImGui::Button("Detect Sprites"))
  {
    std::string fullPath = ResourceManager::Get().GetResourcePath() + sheetPath;
    results = Detect(fullPath, bgColor, colorThreshold, minArea, mergeGap);
    hasResults = true;
    _previewFrame = 0;
  }

  if (!hasResults || results.empty())
  {
    if (hasResults)
      ImGui::Text("No sprites detected. Try adjusting parameters.");
    return false;
  }

  ImGui::Separator();
  ImGui::Text("Detected %d sprites", (int)results.size());

  // Preview navigation
  if (_previewFrame >= (int)results.size())
    _previewFrame = 0;

  if (ImGui::Button("< Prev"))
    _previewFrame = _previewFrame == 0 ? (int)results.size() - 1 : _previewFrame - 1;
  ImGui::SameLine();
  if (ImGui::Button("Next >"))
    _previewFrame = (_previewFrame + 1) % (int)results.size();
  ImGui::SameLine();
  ImGui::SliderInt("##Frame", &_previewFrame, 0, (int)results.size() - 1);

  // --- Per-frame editing ---
  {
    auto& r = results[_previewFrame].bounds;
    ImGui::Text("Frame %d", _previewFrame);

    ImGui::PushItemWidth(200);
    ImGui::DragInt2("Position", &r.x, 1.0f, -4096, 8192);
    ImGui::DragInt2("Size", &r.w, 1.0f, 1, 4096);
    ImGui::PopItemWidth();

    // Expand/shrink helpers — pad the rect uniformly
    ImGui::SameLine();
    if (ImGui::Button("Pad +2"))
    {
      r.x -= 2; r.y -= 2;
      r.w += 4; r.h += 4;
    }
    ImGui::SameLine();
    if (ImGui::Button("Pad +5"))
    {
      r.x -= 5; r.y -= 5;
      r.w += 10; r.h += 10;
    }

    // Show the cropped sprite preview, scaled to fill the preview area
    Rect<float> subRect((float)r.x, (float)r.y, (float)(r.x + r.w), (float)(r.y + r.h));
    int previewHeight = std::clamp(r.h * 3, 128, 512);
    DisplayImage preview(sheetPath, subRect, previewHeight);
    int previewW = std::max(preview.displaySize.x + 20, 200);
    int previewH = std::max(preview.displaySize.y + 20, 200);
    ImGui::BeginChild("SpritePreview", ImVec2((float)previewW, (float)previewH), true);
    preview.Show();
    ImGui::EndChild();
  }

  // Delete / insert controls
  if (ImGui::Button("Delete This Sprite"))
  {
    results.erase(results.begin() + _previewFrame);
    if (_previewFrame >= (int)results.size() && _previewFrame > 0)
      _previewFrame--;
  }

  ImGui::Separator();

  // Frame list with inline editing
  if (ImGui::CollapsingHeader("All Detected Frames"))
  {
    int deleteIdx = -1;
    for (int i = 0; i < (int)results.size(); i++)
    {
      auto& r = results[i].bounds;
      ImGui::PushID(i);

      bool selected = (i == _previewFrame);
      if (ImGui::Selectable(("##frame" + std::to_string(i)).c_str(), selected, 0, ImVec2(0, 20)))
        _previewFrame = i;
      ImGui::SameLine();
      ImGui::Text("%3d: (%4d,%4d) %3dx%3d", i, r.x, r.y, r.w, r.h);

      // Show inline editing when this frame is selected
      if (selected)
      {
        ImGui::Indent(20.0f);
        ImGui::PushItemWidth(180);
        ImGui::DragInt2("Pos##inline", &r.x, 1.0f, -4096, 8192);
        ImGui::DragInt2("Size##inline", &r.w, 1.0f, 1, 4096);
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if (ImGui::Button("Delete##inline"))
          deleteIdx = i;
        ImGui::Unindent(20.0f);
      }

      ImGui::PopID();
    }
    if (deleteIdx >= 0)
    {
      results.erase(results.begin() + deleteIdx);
      if (_previewFrame >= (int)results.size() && _previewFrame > 0)
        _previewFrame--;
    }
  }

  ImGui::Separator();

  // Apply button — transfers results to the section's frameRects
  if (ImGui::Button("Apply to Section"))
  {
    outFrameRects.clear();
    outFrameRects.reserve(results.size());
    for (auto& r : results)
      outFrameRects.push_back(r.bounds);
    applied = true;
  }

  return applied;
}
