#include "Sprite.h"
#include "AssetManagement/Resource.h"

void Sprite::Init(const char* sheet)
{
  //adds new operation to the blitting list
  _blitter = ResourceManager::Get().RegisterBlitOp();

  _blitter->_textureResource = &ResourceManager::Get().GetTexture(sheet);

  _sourceRect.x = 0;
  _sourceRect.y = 0;

  auto sourceImage = ResourceManager::Get().GetRawImage(ResourceManager::Get().GetResourcePath() + sheet);
  _sourceRect.w = sourceImage.Get()->w;
  _sourceRect.h = sourceImage.Get()->h;

  _blitter->_textureRect = _sourceRect;
}

void Sprite::PushToRenderer(const Transform& transform)
{
  _blitter->_displayRect = { transform.position.x, transform.position.y,
    (int)(static_cast<float>(_sourceRect.w) * transform.scale.x), (int)(static_cast<float>(_sourceRect.h) * transform.scale.y) };
}

void Camera::Init(int w, int h)
{
  _rect.x = 0;
  _rect.y = 0;
  _rect.w = w;
  _rect.h = h;
}

void Camera::Update(Transform& transform, float dt)
{
  _rect.x = transform.position.x;
  _rect.y = transform.position.y;
}

void Camera::ConvScreenSpace(ResourceManager::BlitOperation* entity)
{
  entity->_displayRect.x -= _rect.x;
  entity->_displayRect.y -= _rect.y;
}

bool Camera::EntityInDisplay(const ResourceManager::BlitOperation* entity)
{
  return SDLRectOverlap(_rect, entity->_displayRect);
}
