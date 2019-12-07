#include "Sprite.h"
#include "AssetManagement/Resource.h"

void Sprite::Init(const char* sheet)
{
  //adds new operation to the blitting list
  ResourceManager::Get().RegisterBlitOp();

  _textureResource = &ResourceManager::Get().GetTexture(sheet);

  _sourceRect.x = 0;
  _sourceRect.y = 0;
  SDL_QueryTexture(_textureResource->Get(), nullptr, nullptr, &_sourceRect.w, &_sourceRect.h);
}

void Sprite::OnFrameBegin()
{
  _op = ResourceManager::Get().GetAvailableOp();
}

void Sprite::Update(float dt)
{
  _op->_textureRect = _sourceRect;
  _op->_textureResource = _textureResource;

  Transform& transform = _owner->transform;
  _op->_displayRect = { static_cast<int>(std::floorf(transform.position.x)), static_cast<int>(std::floorf(transform.position.y)),
    (int)(static_cast<float>(_sourceRect.w) * transform.scale.x),
    (int)(static_cast<float>(_sourceRect.h) * transform.scale.y) };

  _op->valid = true;
}

void Camera::Init(int w, int h)
{
  _rect.x = 0;
  _rect.y = 0;
  _rect.w = w;
  _rect.h = h;
}

void Camera::Update(float dt)
{
  _rect.x = static_cast<int>(std::floorf(_owner->transform.position.x));
  _rect.y = static_cast<int>(std::floorf(_owner->transform.position.y));

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
