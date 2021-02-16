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

class UIBoxSpriteRenderComponent : public IComponent
{
public:
  UIBoxSpriteRenderComponent() : IComponent() {}
  void OnAdd(const EntityID& entity) override;
  void OnRemove(const EntityID& entity) override;

  void Init(Resource<GLTexture>& resource, int columns, int rows, int tlIdx, int trIdx, int blIdx, int brIdx, int lSideIdx, int tSideIdx, int rSideIdx, int bSideIdx)
  {
    _resource = std::shared_ptr<ResourceWrapper<GLTexture>>(new ResourceWrapper<GLTexture>(resource));

    _texWidth = _resource->GetResource()->w();
    _texHeight = _resource->GetResource()->h();

    _rows = rows;

    _spriteWidth = static_cast<float>(_texWidth) / static_cast<float>(columns);
    _spriteHeight = static_cast<float>(_texHeight) / static_cast<float>(rows);

    _tl = tlIdx;
    _tr = trIdx;
    _bl = blIdx;
    _br = brIdx;
    _l = lSideIdx;
    _t = tSideIdx;
    _r = rSideIdx;
    _b = bSideIdx;

  }

  Resource<GLTexture>* GetTexture() { return &_resource->GetResource(); }

  DrawRect<float> GetSpritePart(int part)
  {
    int idx = 0;
    switch (part)
    {
    case 0: idx = _tl; break;
    case 1: idx = _t; break;
    case 2: idx = _tr; break;
    case 3: idx = _l; break;
    case 4: idx = _r; break;
    case 5: idx = _bl; break;
    case 6: idx = _b; break;
    case 7: idx = _br; break;
    default: idx = 0;
    }

    int x = idx % _rows;
    int y = idx / _rows;

    return DrawRect<float>(x * _spriteWidth, y * _spriteHeight, _spriteWidth, _spriteHeight);
  }

private:
  std::shared_ptr<ResourceWrapper<GLTexture>> _resource = nullptr;
  int _texWidth = 0;
  int _texHeight = 0;

  int _rows = 0;

  float _spriteWidth = 0;
  float _spriteHeight = 0;

  // sprite sheet index locations for parts of box
  int _tl, _tr, _bl, _br, _l, _t, _r, _b = 0;

};
