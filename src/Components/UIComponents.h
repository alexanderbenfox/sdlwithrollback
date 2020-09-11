#pragma once
#include "Core/ECS/IComponent.h"
#include "Components/StateComponent.h"
#include "AssetManagement/BlitOperation.h"
#include "Rendering/RenderManager.h"

enum class UIAnchor
{
  TL, TR, BL, BR, Center, Size
};

//______________________________________________________________________________
//!
class UITransform : public Transform
{
public:
  // point to move relative to on the parent (if no parent, then the entire screen)
  UIAnchor anchor;

  UITransform* parent = nullptr;

  Vector2<float> screenPosition;

};

struct UIContainer : public IComponent
{
  // empty callback function
  typedef std::function<void(std::shared_ptr<Entity>, const StateComponent*, const StateComponent*)>  UIUpdateFunction;

  struct Updater
  {
    std::shared_ptr<Entity> uiElementEntity;
    UIUpdateFunction callback;
  };

  std::vector<Updater> uiUpdaters;
  // last state of the parent
  StateComponent lastState;
};

class UIRectangleRenderComponent : public IComponent
{
public:
  UIRectangleRenderComponent();
  void OnAdd(const EntityID& entity) override;
  void OnRemove(const EntityID& entity) override;
  DrawRect<float> shownSize;
  bool isFilled = false;

};
