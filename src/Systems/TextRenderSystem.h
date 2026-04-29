#pragma once
#include "Components/RenderComponent.h"
#include "Rendering/RenderManager.h"

namespace TextRenderSystem
{
  //! Sets up draw ops for text rendering and returns the size of the on-screen text field
  inline Vector2<float> SetText(TextRenderer& tr, const std::string& text, TextAlignment alignment, int fieldWidth = 600)
  {
    Vector2<float> newSize;
    if (tr._resource && text != tr._currentText)
    {
      // remove the old operations from draw manager
      for (size_t i = 0; i < tr._string.size(); ++i)
        RenderManager::Get().DeregisterDrawable<BlitOperation<RenderType>>(RenderLayer::UI);

      tr._currentText = text;
      tr._string = tr._resource->CreateStringField(tr._currentText.c_str(), fieldWidth, alignment);

      float width = 0;
      float height = 0;

      // for each letter, register a new blit op with the renderer
      for (auto& letter : tr._string)
      {
        RenderManager::Get().RegisterDrawable<BlitOperation<RenderType>>(RenderLayer::UI);
        width = std::max(letter.x + letter.texture->GetInfo().mWidth, width);
        height = std::max(letter.y + letter.texture->GetInfo().mHeight, height);
      }
      newSize = Vector2<float>(width, height);
    }
    return newSize;
  }

  //! Deregisters all draw ops for this TextRenderer from the RenderManager
  inline void Cleanup(TextRenderer& tr)
  {
    for (size_t i = 0; i < tr._string.size(); ++i)
      RenderManager::Get().DeregisterDrawable<BlitOperation<RenderType>>(RenderLayer::UI);
  }
}
