#pragma once
#include "Components/RenderComponent.h"

namespace TextRenderSystem
{
  //! Sets up draw ops for text rendering and returns the size of the on-screen text field
  inline Vector2<float> SetText(TextRenderer& tr, const std::string& text, TextAlignment alignment, int fieldWidth = 600)
  {
    Vector2<float> newSize;
    if (tr._resource && text != tr._currentText)
    {
      tr._currentText = text;
      tr._string = tr._resource->CreateStringField(tr._currentText.c_str(), fieldWidth, alignment);

      float width = 0;
      float height = 0;

      for (auto& letter : tr._string)
      {
        width = std::max(letter.x + letter.texture->GetInfo().mWidth, width);
        height = std::max(letter.y + letter.texture->GetInfo().mHeight, height);
      }
      newSize = Vector2<float>(width, height);
    }
    return newSize;
  }
}
