#pragma once
#include "Components/IComponent.h"
#include "Components/StateComponent.h"
#include "AssetManagement/BlitOperation.h"
#include "Rendering/RenderManager.h"

enum class UIAnchor
{
  TL, TR, BL, BR, Center, Size
};

//! Components that need to 
struct UIComponent
{
  virtual void TransferState(const StateComponent* stateInfo)
  {
    callback(&lastState, stateInfo, this);
    lastState = *stateInfo;
  }
  // empty callback function
  std::function<void(const StateComponent*, const StateComponent*, UIComponent*)> callback = [](const StateComponent*, const StateComponent*,UIComponent*){};
  StateComponent lastState;
};

//______________________________________________________________________________
//!
class UITransform : public Transform, public UIComponent
{
public:
  UITransform(std::shared_ptr<Entity> owner) : Transform(owner) {}

  // point to move relative to on the parent (if no parent, then the entire screen)
  UIAnchor anchor;

  std::shared_ptr<UITransform> parent;

  Vector2<float> screenPosition;

};

class UIContainer : public IComponent
{
public:
  UIContainer(std::shared_ptr<Entity> owner) : IComponent(owner) {}
  std::vector<std::shared_ptr<UIComponent>> uiComponents;
};

class UIRectangleRenderComponent : public IComponent, public UIComponent
{
public:
  UIRectangleRenderComponent(std::shared_ptr<Entity> owner);
  virtual ~UIRectangleRenderComponent();
  DrawRect<float> shownSize;
  bool isFilled = false;

};
