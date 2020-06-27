#pragma once
#include "Systems/ISystem.h"
#include "GameManagement.h"

#include "Components/UIComponents.h"

const Rect<float> ScreenRect(0, 0, m_nativeWidth, m_nativeHeight);

class UIPositionUpdateSystem : public ISystem<UITransform>
{
public:
  static void CalcScreenPos(UITransform* transform, Rect<float> parentRect, int x, int y)
  {
    switch(transform->anchor)
    {
      case UIAnchor::TR:
        transform->screenPosition = Vector2<float>(parentRect.end.x + x, parentRect.beg.y + y);
        break;
      case UIAnchor::BL:
        transform->screenPosition  = Vector2<float>(parentRect.beg.x + x, parentRect.end.y + y);
        break;
      case UIAnchor::BR:
        transform->screenPosition  = Vector2<float>(parentRect.end.x + x, parentRect.end.y + y);
        break;
      case UIAnchor::Center:
        transform->screenPosition  = Vector2<float>(parentRect.GetCenter().x - transform->rect.HalfWidth(), parentRect.GetCenter().y - transform->rect.HalfHeight());
        break;
      case UIAnchor::TL:
      default:
        transform->screenPosition  = Vector2<float>(parentRect.beg.x + x, parentRect.beg.y + y);
        break;
    }
  }

  static void DoTick(float dt)
  {
    for (auto tuple : Tuples)
    {
      UITransform* transform = std::get<UITransform*>(tuple.second);
      float x = transform->position.x;
      float y = transform->position.y;
      CalcScreenPos(transform, 
        transform->parent ? 
        Rect<float>(transform->parent->screenPosition.x, transform->parent->screenPosition.y, transform->parent->rect.Width(), transform->parent->rect.Height()) : ScreenRect,
        x, y);
    }
  }
};

class UIContainerUpdateSystem : public ISystem<UIContainer, StateComponent>
{
public:
  static void DoTick(float dt)
  {
    for (auto tuple : Tuples)
    {
      UIContainer* container = std::get<UIContainer*>(tuple.second);
      StateComponent* info = std::get<StateComponent*>(tuple.second);

      for (auto item : container->uiComponents)
      {
        item->TransferState(info);
      }
    }
  }
};
