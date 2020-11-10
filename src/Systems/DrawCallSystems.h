#pragma once
#include "Core/ECS/ISystem.h"
#include "Components/RenderComponent.h"
#include "Components/UIComponents.h"
#include "Managers/GameManagement.h"

//! System that creates draw calls for textured sprites and passes them to the renderer
class SpriteDrawCallSystem : public ISystem<Transform, RenderComponent<RenderType>, RenderProperties>
{
public:
  static void PostUpdate()
  {
    for (const EntityID& entity : Registered)
    {
      RenderComponent<RenderType>& renderer = ComponentArray<RenderComponent<RenderType>>::Get().GetComponent(entity);
      Transform& transform = ComponentArray<Transform>::Get().GetComponent(entity);
      RenderProperties& properties = ComponentArray<RenderProperties>::Get().GetComponent(entity);

      // if the render resource hasn't been assigned yet, hold off
      if (!renderer.GetRenderResource()) continue;

      // get a display op to set draw parameters
      auto displayOp = GRenderer.GetAvailableOp<BlitOperation<RenderType>>(RenderLayer::World);

      displayOp->srcRect = renderer.sourceRect;
      displayOp->textureResource = renderer.GetRenderResource();

      // get scaled rect transform to scale between texture space and game space
      Vector2<float> scaledRectTransform = properties.rectTransform / properties.renderScaling;
      Vector2<float> renderOffset = CalculateRenderOffset(properties.anchor, properties.offset, scaledRectTransform);
      if (properties.horizontalFlip)
        renderOffset.x = -renderer.sourceRect.w - renderOffset.x;// +scaledRectTransform.x;

      renderOffset *= properties.renderScaling;
      Vector2<float> targetPos(transform.position.x + renderOffset.x * transform.scale.x, transform.position.y + renderOffset.y * transform.scale.y);

      displayOp->targetRect = DrawRect<float>(targetPos.x, targetPos.y,
        renderer.sourceRect.w * transform.scale.x * properties.renderScaling.x,
        renderer.sourceRect.h * transform.scale.y * properties.renderScaling.y);

      // set properties
      displayOp->flip = properties.horizontalFlip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
      // set display color directly
      displayOp->displayColor = properties.GetDisplayColor();

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
    for (const EntityID& entity : Registered)
    {
      TextRenderer& renderer = ComponentArray<TextRenderer>::Get().GetComponent(entity);
      RenderProperties& properties = ComponentArray<RenderProperties>::Get().GetComponent(entity);
      UITransform& transform = ComponentArray<UITransform>::Get().GetComponent(entity);

      Vector2<float> displayPosition = transform.screenPosition;

      for (GLDrawOperation& drawOp : renderer.GetRenderOps())
      {
        // get a display op to set draw parameters
        auto displayOp = GRenderer.GetAvailableOp<BlitOperation<RenderType>>(RenderLayer::UI);

        displayOp->srcRect = DrawRect<float>(0, 0, (*drawOp.texture)->w(), (*drawOp.texture)->h());
        displayOp->textureResource = drawOp.texture;

        Vector2<int> drawOffset(drawOp.x, drawOp.y);
        Vector2<int> renderOffset = CalculateRenderOffset(properties.anchor, properties.offset, properties.rectTransform) + drawOffset;

        displayOp->targetRect = DrawRect<float>(displayPosition.x + renderOffset.x * transform.scale.x,
          displayPosition.y + renderOffset.y * transform.scale.y,
          displayOp->srcRect.w * transform.scale.x,
          displayOp->srcRect.h * transform.scale.y);

        // set properties
        displayOp->flip = properties.horizontalFlip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
        // set display color directly
        displayOp->displayColor = properties.GetDisplayColor();

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
    for (const EntityID& entity : Registered)
    {
      UIRectangleRenderComponent& uiElement = ComponentArray<UIRectangleRenderComponent>::Get().GetComponent(entity);
      RenderProperties& properties = ComponentArray<RenderProperties>::Get().GetComponent(entity);
      UITransform& transform = ComponentArray<UITransform>::Get().GetComponent(entity);

      auto processOps = [&uiElement, &transform, &properties](DrawPrimitive<RenderType>* displayOp)
      {
        displayOp->targetRect.x += transform.screenPosition.x;
        displayOp->targetRect.y += transform.screenPosition.y;

        // set properties
        displayOp->flip = properties.horizontalFlip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
        // set display color directly
        displayOp->displayColor = properties.GetDisplayColor();

        displayOp->valid = true;
      };

      // get a display op to set draw parameters
      auto op = GRenderer.GetAvailableOp<DrawPrimitive<RenderType>>(RenderLayer::UI);

      op->targetRect = uiElement.shownSize;
      op->filled = uiElement.isFilled;

      processOps(op);
    }
  }
};
