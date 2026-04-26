#pragma once
#include "Core/Geometry2D/Rect.h"
#include "Core/Math/Vector2.h"

#include <string>
#include <vector>

//______________________________________________________________________________
//! Detects individual sprite bounding boxes in a spritesheet image by finding
//! connected components of non-background pixels.
class SpriteDetector
{
public:
  struct Result
  {
    DrawRect<int> bounds;
    int pixelCount = 0;
  };

  //! Run detection on an image file. Returns detected sprite rects sorted
  //! in reading order (top-to-bottom, left-to-right).
  //! @param imagePath  Absolute path to the spritesheet image
  //! @param bgColor    Background color to exclude (default white)
  //! @param colorThreshold  Max per-channel distance from bgColor to count as background
  //! @param minArea    Minimum bounding-box area to keep (filters text/noise)
  //! @param mergeGap   Merge bounding boxes that are within this many pixels of each other
  std::vector<Result> Detect(const std::string& imagePath,
                             SDL_Color bgColor = {255, 255, 255, 255},
                             int colorThreshold = 20,
                             int minArea = 200,
                             int mergeGap = 2);

  //! Display an ImGui window for reviewing and adjusting detected sprites,
  //! then transferring them into a frameRects vector.
  //! Returns true if the user pressed "Apply" to accept results.
  bool DisplayDetectionUI(const std::string& sheetPath,
                          std::vector<DrawRect<int>>& outFrameRects);

  // --- Configuration (exposed for the ImGui UI) ---
  SDL_Color bgColor = {255, 255, 255, 255};
  int colorThreshold = 20;
  int minArea = 400;
  int mergeGap = 2;

  // --- State ---
  std::vector<Result> results;
  bool hasResults = false;

private:
  //! Flood-fill from (startX, startY) marking visited pixels,
  //! returns the bounding box and pixel count of the connected component.
  Result FloodFill(const Uint8* pixels, int w, int h, int bpp, int pitch,
                   int startX, int startY,
                   std::vector<bool>& visited,
                   SDL_Color bg, int threshold) const;

  //! Check if a pixel at (x,y) is foreground (not background)
  bool IsForeground(const Uint8* pixels, int x, int y, int bpp, int pitch,
                    SDL_Color bg, int threshold) const;

  //! Merge overlapping or nearby bounding boxes
  void MergeNearby(std::vector<Result>& rects, int gap) const;

  //! Sort results in reading order
  void SortReadingOrder(std::vector<Result>& rects) const;

  // UI state
  int _previewFrame = 0;
};
