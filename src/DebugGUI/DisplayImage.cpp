#include "DisplayImage.h"
#include "Managers/ResourceManager.h"
#include "../imgui/imgui.h"

DisplayImage::DisplayImage(const std::string& imgPath, Rect<float> imgSubRect, int targetDisplayHeight)
{
  Resource<GLTexture>& resource = ResourceManager::Get().GetAsset<GLTexture>(imgPath);
  Vector2<float> textureSize = ResourceManager::Get().GetTextureWidthAndHeight(imgPath);

  Rect<float> uvRect(imgSubRect.beg / textureSize, imgSubRect.end / textureSize);

  int displayWidth = imgSubRect.Width() / imgSubRect.Height() * targetDisplayHeight;

  ptr = (void*)(intptr_t)resource.GetConst()->ID();
  displaySize = Vector2<int>(displayWidth, targetDisplayHeight);
  uv0 = uvRect.beg;
  uv1 = uvRect.end;
}

Vector2<float> DisplayImage::Show() const
{
  ImVec2 currPos = ImGui::GetWindowPos();
  if (ptr)
    ImGui::Image((void*)(intptr_t)ptr, ImVec2(displaySize.x, displaySize.y), ImVec2(uv0.x, uv0.y), ImVec2(uv1.x, uv1.y));
  return Vector2<float>(currPos.x, currPos.y);
}
