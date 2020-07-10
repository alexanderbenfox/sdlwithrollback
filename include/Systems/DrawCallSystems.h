#pragma once
#include "Systems/ISystem.h"
#include "Components/RenderComponent.h"
#include "Components/UIComponents.h"
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
      auto displayOp = GRenderer.GetAvailableOp<BlitOperation<RenderType>>(RenderLayer::World);

      displayOp->textureRect = renderer->sourceRect;
      displayOp->textureResource = renderer->GetRenderResource();

      Vector2<int> renderOffset = properties->Offset();

      displayOp->displayRect = OpSysConv::CreateSDLRect(
        static_cast<int>(std::floor(transform->position.x + renderOffset.x * transform->scale.x)),
        static_cast<int>(std::floor(transform->position.y + renderOffset.y * transform->scale.y)),
        (int)(static_cast<float>(renderer->sourceRect.w) * transform->scale.x),
        (int)(static_cast<float>(renderer->sourceRect.h) * transform->scale.y));

      // set properties
      displayOp->flip = properties->horizontalFlip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
      // set display color directly
      displayOp->displayColor = properties->GetDisplayColor();

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
      Vector2<float> displayPosition = transform->screenPosition;

      for (GLDrawOperation& drawOp : renderer->GetRenderOps())
      {
        // get a display op to set draw parameters
        auto displayOp = GRenderer.GetAvailableOp<BlitOperation<RenderType>>(RenderLayer::UI);

        const SDL_Rect sourceRect = { 0, 0, (*drawOp.texture)->w(), (*drawOp.texture)->h() };

        displayOp->textureRect = sourceRect;
        displayOp->textureResource = drawOp.texture;

        Vector2<int> drawOffset(drawOp.x, drawOp.y);
        Vector2<int> renderOffset = properties->Offset() + drawOffset;

        displayOp->displayRect = OpSysConv::CreateSDLRect(
          static_cast<int>(std::floor(displayPosition.x + renderOffset.x * transform->scale.x)),
          static_cast<int>(std::floor(displayPosition.y + renderOffset.y * transform->scale.y)),
          (int)(static_cast<float>(sourceRect.w) * transform->scale.x),
          (int)(static_cast<float>(sourceRect.h) * transform->scale.y));

        // set properties
        displayOp->flip = properties->horizontalFlip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
        // set display color directly
        displayOp->displayColor = properties->GetDisplayColor();

        displayOp->valid = true;
      }
    }
  }
};

class DrawUIPrimitivesSystem : public ISystem<UITransform, UIRectangleRenderComponent, RenderProperties>
{
public:
  static void PostUpdate()
  {
    for (auto tuple : Tuples)
    {
      UIRectangleRenderComponent* uiElement = std::get<UIRectangleRenderComponent*>(tuple.second);
      RenderProperties* properties = std::get<RenderProperties*>(tuple.second);
      UITransform* transform = std::get<UITransform*>(tuple.second);

      auto processOps = [uiElement, transform, properties](DrawPrimitive<RenderType>* displayOp)
      {
        displayOp->displayRect.x += transform->screenPosition.x;
        displayOp->displayRect.y += transform->screenPosition.y;

        // set properties
        displayOp->flip = properties->horizontalFlip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
        // set display color directly
        displayOp->displayColor = properties->GetDisplayColor();

        displayOp->valid = true;
      };

      // get a display op to set draw parameters
      auto op = GRenderer.GetAvailableOp<DrawPrimitive<RenderType>>(RenderLayer::UI);

      op->displayRect = uiElement->shownSize;
      op->filled = uiElement->isFilled;

      processOps(op);
    }
  }
};
