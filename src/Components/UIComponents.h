#pragma once
#include "Core/ECS/IComponent.h"
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
  // point to move relative to on the parent (if no parent, then the entire screen)
  UIAnchor anchor;

  UITransform* parent = nullptr;

  Vector2<float> screenPosition;

};

struct UIContainer : public IComponent
{
  std::vector<UIComponent*> uiComponents;
};

class UIRectangleRenderComponent : public IComponent, public UIComponent
{
public:
  UIRectangleRenderComponent();
  void OnAdd(const EntityID& entity) override;
  void OnRemove(const EntityID& entity) override;
  DrawRect<float> shownSize;
  bool isFilled = false;

};
