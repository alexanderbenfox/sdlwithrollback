#pragma once
#include "Core/ECS/ISystem.h"
#include "Components/RenderComponent.h"
#include "Components/UIComponents.h"
#include "Rendering/RenderManager.h"

//! System that creates draw calls for textured sprites and passes them to the renderer
class SpriteDrawCallSystem : public ISystem<Transform, RenderComponent<RenderType>, RenderProperties>
{
public:
  static void Check(Entity* entity)
  {
    bool wasMember = Registered.count(entity->GetID());
    ISystem::Check(entity);
    bool isMember = Registered.count(entity->GetID());
    if (!wasMember && isMember)
      RenderManager::Get().RegisterDrawable<BlitOperation<RenderType>>(RenderLayer::World);
    else if (wasMember && !isMember)
      RenderManager::Get().DeregisterDrawable<BlitOperation<RenderType>>(RenderLayer::World);
  }

  static void PostUpdate()
  {
    PROFILE_FUNCTION();
    for (const EntityID& entity : Registered)
    {
      RenderComponent<RenderType>& renderer = ComponentArray<RenderComponent<RenderType>>::Get().GetComponent(entity);
      Transform& transform = ComponentArray<Transform>::Get().GetComponent(entity);
      RenderProperties& properties = ComponentArray<RenderProperties>::Get().GetComponent(entity);

      // if the render resource hasn't been assigned yet, hold off
      if (!renderer.GetRenderResource()) continue;

      // get a display op to set draw parameters
      auto displayOp = RenderManager::Get().GetAvailableOp<BlitOperation<RenderType>>(RenderLayer::World);

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
      displayOp->flip = properties.horizontalFlip ? FlipMode::Horizontal : FlipMode::None;
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
    PROFILE_FUNCTION();
    for (const EntityID& entity : Registered)
    {
      TextRenderer& renderer = ComponentArray<TextRenderer>::Get().GetComponent(entity);
      RenderProperties& properties = ComponentArray<RenderProperties>::Get().GetComponent(entity);
      UITransform& transform = ComponentArray<UITransform>::Get().GetComponent(entity);

      Vector2<float> displayPosition = transform.screenPosition;

      for (TextDrawOp& drawOp : renderer.GetRenderOps())
      {
        // get a display op to set draw parameters
        auto displayOp = RenderManager::Get().GetAvailableOp<BlitOperation<RenderType>>(RenderLayer::UI);

        displayOp->srcRect = DrawRect<float>(0, 0, (*drawOp.texture)->w(), (*drawOp.texture)->h());
        displayOp->textureResource = drawOp.texture;

        Vector2<int> drawOffset(drawOp.x, drawOp.y);
        Vector2<int> renderOffset = CalculateRenderOffset(properties.anchor, properties.offset, properties.rectTransform) + drawOffset;

        displayOp->targetRect = DrawRect<float>(displayPosition.x + renderOffset.x * transform.scale.x,
          displayPosition.y + renderOffset.y * transform.scale.y,
          displayOp->srcRect.w * transform.scale.x,
          displayOp->srcRect.h * transform.scale.y);

        // set properties
        displayOp->flip = properties.horizontalFlip ? FlipMode::Horizontal : FlipMode::None;
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
  static void Check(Entity* entity)
  {
    bool wasMember = Registered.count(entity->GetID());
    ISystem::Check(entity);
    bool isMember = Registered.count(entity->GetID());
    if (!wasMember && isMember)
    {
      RenderManager::Get().RegisterDrawable<DrawPrimitive<RenderType>>(RenderLayer::UI);
      UIRectangleRenderComponent& uiElement = ComponentArray<UIRectangleRenderComponent>::Get().GetComponent(entity->GetID());
      UITransform& transform = ComponentArray<UITransform>::Get().GetComponent(entity->GetID());
      uiElement.shownSize.w = transform.rect.Width();
      uiElement.shownSize.h = transform.rect.Height();
    }
    else if (wasMember && !isMember)
      RenderManager::Get().DeregisterDrawable<DrawPrimitive<RenderType>>(RenderLayer::UI);
  }

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
        displayOp->flip = properties.horizontalFlip ? FlipMode::Horizontal : FlipMode::None;
        // set display color directly
        displayOp->displayColor = properties.GetDisplayColor();

        displayOp->valid = true;
      };

      // get a display op to set draw parameters
      auto op = RenderManager::Get().GetAvailableOp<DrawPrimitive<RenderType>>(RenderLayer::UI);

      op->targetRect = uiElement.shownSize;
      op->filled = uiElement.isFilled;

      processOps(op);
    }
  }
};

class DrawUIBoxSpriteSystem : public ISystem<UITransform, UIBoxSpriteRenderComponent, RenderProperties>
{
public:
  static void Check(Entity* entity)
  {
    bool wasMember = Registered.count(entity->GetID());
    ISystem::Check(entity);
    bool isMember = Registered.count(entity->GetID());
    if (!wasMember && isMember)
    {
      for (int i = 0; i < 8; i++)
        RenderManager::Get().RegisterDrawable<BlitOperation<RenderType>>(RenderLayer::UI);
    }
    else if (wasMember && !isMember)
    {
      for (int i = 0; i < 8; i++)
        RenderManager::Get().DeregisterDrawable<BlitOperation<RenderType>>(RenderLayer::UI);
    }
  }

  static void PostUpdate()
  {
    for (const EntityID& entity : Registered)
    {
      UIBoxSpriteRenderComponent& boxSprite = ComponentArray<UIBoxSpriteRenderComponent>::Get().GetComponent(entity);
      RenderProperties& properties = ComponentArray<RenderProperties>::Get().GetComponent(entity);
      UITransform& transform = ComponentArray<UITransform>::Get().GetComponent(entity);

      DrawRect<float> totalDraw(transform.screenPosition.x, transform.screenPosition.y, transform.rect.Width(), transform.rect.Height());
      float partW = (totalDraw.w / 3.0f);
      float partH = (totalDraw.h / 3.0f);

      for (int i = 0; i < 8; i++)
      {
        // get a display op to set draw parameters
        auto op = RenderManager::Get().GetAvailableOp<BlitOperation<RenderType>>(RenderLayer::UI);

        op->textureResource = boxSprite.GetTexture();
        op->srcRect = boxSprite.GetSpritePart(i);

        op->flip = FlipMode::None;

        op->displayColor = properties.GetDisplayColor();

        int drawIdx = i;
        if (i >= 4)
          drawIdx = drawIdx + 1;

        Vector2<float> start((drawIdx % 3) * partW + totalDraw.x, (drawIdx / 3) * partH + totalDraw.y);
        op->targetRect = DrawRect<float>(start.x, start.y, partW, partH);

        op->valid = true;
      }
    }
  }
};
