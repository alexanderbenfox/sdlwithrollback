#include "Components/RenderComponent.h"
#include "Components/UIComponents.h"

RenderProperties::RenderProperties() :
  rectTransform(0, 0),
  offset(0, 0),
  horizontalFlip(false),
  _displayColor{ 255, 255, 255, 255 },
  IComponent()
{}

void RenderProperties::SetDisplayColor(uint8_t r, uint8_t g, uint8_t b)
{
  _displayColor = { r, g, b, 255 };
}
void RenderProperties::SetDisplayColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
  _displayColor = { r, g, b, a };
}

Color RenderProperties::GetDisplayColor() const
{
  return _displayColor;
}
