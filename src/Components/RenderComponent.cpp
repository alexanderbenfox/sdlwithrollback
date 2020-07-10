#include "Components/RenderComponent.h"
#include "Components/UIComponents.h"

TextRenderer::TextRenderer(std::shared_ptr<Entity> owner) : _resource(nullptr), _currentText(""), IComponent(owner) {}

TextRenderer::~TextRenderer()
{
  // make sure that the blit operations have been cleared
  for (size_t i = 0; i < _string.size(); ++i)
    RenderManager<GLTexture>::Get().DeregisterDrawable<BlitOperation<GLTexture>>(RenderLayer::UI);
}

void TextRenderer::SetFont(LetterCase& resource)
{
  _resource = &resource;
}

void TextRenderer::SetText(const std::string& text)
{
  if (_resource && text != _currentText)
  {
    // remove the operations from draw manager
    for (size_t i = 0; i < _string.size(); ++i)
      RenderManager<GLTexture>::Get().DeregisterDrawable<BlitOperation<GLTexture>>(RenderLayer::UI);

    _currentText = text;
    _string = _resource->CreateStringField(_currentText.c_str(), 600);

    float width = 0;
    float height = 0;

    // for each letter, we will have to send a new blit op to the renderer manager
    for (auto& letter : _string)
    {
      RenderManager<GLTexture>::Get().RegisterDrawable<BlitOperation<GLTexture>>(RenderLayer::UI);

      width = std::max(letter.x + letter.texture->GetInfo().mWidth, width);
      height = std::max(letter.y + letter.texture->GetInfo().mHeight, height);
    }
    if (auto transform = _owner->GetComponent<UITransform>())
    {
      transform->rect = Rect<float>(transform->position.x, transform->position.y, width, height);
    }
  }
}

std::vector<GLDrawOperation> TextRenderer::GetRenderOps()
{
  return _string;
}

RenderProperties::RenderProperties(std::shared_ptr<Entity> owner) :
  baseRenderOffset(0, 0),
  offset(0, 0),
  horizontalFlip(false),
  _displayColor{ 255, 255, 255, SDL_ALPHA_OPAQUE },
  IComponent(owner)
{}

void RenderProperties::SetDisplayColor(Uint8 r, Uint8 g, Uint8 b)
{
  _displayColor = { r, g, b, SDL_ALPHA_OPAQUE };
}
void RenderProperties::SetDisplayColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
  _displayColor = { r, g, b, a };
}

SDL_Color RenderProperties::GetDisplayColor() const
{
  return _displayColor;
}

Vector2<int> RenderProperties::Offset() const
{
  return baseRenderOffset + offset;
}
