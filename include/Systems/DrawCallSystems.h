#pragma once
#include "Systems/ISystem.h"
#include "Components/RenderComponent.h"
#include "GameManagement.h"

//! System that creates draw calls for textured sprites and passes them to the renderer
class SpriteDrawCallSystem : public ISystem<Transform, RenderComponent<RenderType>, RenderProperties>
{
public:
  static void PostUpdate()
  {
    for (auto tuple : Tuples)
    {
      RenderComponent<RenderType>* renderer = std::get<RenderComponent<RenderType>*>(tuple.second);
      Transform* transform = std::get<Transform*>(tuple.second);
      RenderProperties* properties = std::get<RenderProperties*>(tuple.second);

      // if the render resource hasn't been assigned yet, hold off
      if (!renderer->GetRenderResource()) continue;

      // get a display op to set draw parameters
      auto displayOp = GRenderer.GetAvailableOp();

      displayOp->_textureRect = renderer->sourceRect;
      displayOp->_textureResource = renderer->GetRenderResource();

      Vector2<int> renderOffset = properties->Offset();

      displayOp->_displayRect = OpSysConv::CreateSDLRect(
        static_cast<int>(std::floor(transform->position.x + renderOffset.x * transform->scale.x)),
        static_cast<int>(std::floor(transform->position.y + renderOffset.y * transform->scale.y)),
        (int)(static_cast<float>(renderer->sourceRect.w) * transform->scale.x),
        (int)(static_cast<float>(renderer->sourceRect.h) * transform->scale.y));

      // set properties
      displayOp->_flip = properties->horizontalFlip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
      // set display color directly
      displayOp->_displayColor = properties->GetDisplayColor();

      displayOp->valid = true;
    }
  }
};

//! System that creates draw calls for Letter cased textures (or any other multi-texture container object) and passes them to the renderer
class UITextDrawCallSystem : public ISystem<UITransform, TextRenderer, RenderProperties>
{
public:
  static void PostUpdate()
  {
    for (auto tuple : Tuples)
    {
      TextRenderer* renderer = std::get<TextRenderer*>(tuple.second);
      RenderProperties* properties = std::get<RenderProperties*>(tuple.second);
      UITransform* transform = std::get<UITransform*>(tuple.second);

      Vector2<float> size = renderer->GetStringSize();
      Vector2<float> displayPosition = transform->position;
      float x = displayPosition.x;
      float y = displayPosition.y;

      switch(transform->screenAnchor)
      {
        case UIAnchor::TR:
          displayPosition = Vector2<float>(m_nativeWidth + x, y);
          break;
        case UIAnchor::BL:
          displayPosition = Vector2<float>(x, m_nativeHeight + y);
          break;
        case UIAnchor::BR:
          displayPosition = Vector2<float>(m_nativeWidth + x, m_nativeHeight + y);
          break;
        case UIAnchor::Center:
          displayPosition = Vector2<float>(((float)m_nativeWidth - size.x) / 2.0f,
                                                ((float)m_nativeHeight - size.y) / 2.0f);
          break;
        case UIAnchor::TL:
        default:
          displayPosition = Vector2<float>(x, y);
          break;
      }

      for (GLDrawOperation& drawOp : renderer->GetRenderOps())
      {
        BlitOperation<GLTexture> displayOp;

        const SDL_Rect sourceRect = { 0, 0, (*drawOp.texture)->w(), (*drawOp.texture)->h() };

        displayOp._textureRect = sourceRect;
        displayOp._textureResource = drawOp.texture;

        Vector2<int> drawOffset(drawOp.x, drawOp.y);
        Vector2<int> renderOffset = properties->Offset() + drawOffset;

        displayOp._displayRect = OpSysConv::CreateSDLRect(
          static_cast<int>(std::floor(displayPosition.x + renderOffset.x * transform->scale.x)),
          static_cast<int>(std::floor(displayPosition.y + renderOffset.y * transform->scale.y)),
          (int)(static_cast<float>(sourceRect.w) * transform->scale.x),
          (int)(static_cast<float>(sourceRect.h) * transform->scale.y));

        // set properties
        displayOp._flip = properties->horizontalFlip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
        // set display color directly
        displayOp._displayColor = properties->GetDisplayColor();

        displayOp.valid = true;

        // specifically only append to gl render manager
        RenderManager<GLTexture>::Get().AppendDrawOp(std::move(displayOp));
      }
    }
  }
};
