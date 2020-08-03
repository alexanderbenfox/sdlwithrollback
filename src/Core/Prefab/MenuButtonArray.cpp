#include "Core/Prefab/MenuButtonArray.h"

#include "Components/Transform.h"
#include "Components/UIComponents.h"
#include "Components/RenderComponent.h"

#include "Systems/MenuSystem.h"
#include "Systems/DestroyEntitiesSystem.h"

#include "GameManagement.h"

MenuButtonArray::MenuButtonArray(int columns, int rows, float margin) : _size(columns, rows)
{
  _cellSize = Vector2<float>((float)m_nativeWidth / (float)_size.x, (float)m_nativeHeight / (float)_size.y);
  _margin = Vector2<float>(_cellSize.x * margin, _cellSize.y * margin);
}
  
void MenuButtonArray::CreateMenuOption(const char* label, std::function<void()> callback, Vector2<int> position)
{
  auto option = GameManager::Get().CreateEntity<UITransform, TextRenderer, RenderProperties, MenuItem, DestroyOnSceneEnd>();
  option->GetComponent<UITransform>()->anchor = UIAnchor::TL;

  // right now just make the menu cover the entire screen
  option->GetComponent<UITransform>()->rect = Rect<float>(0, 0, _cellSize.x - 2.0f * _margin.x, _cellSize.y - 2.0f * _margin.y);
  option->GetComponent<UITransform>()->position = Vector2<float>(_cellSize.x * position.x + _margin.x, _cellSize.y * position.y + _margin.y);

  // add after setting the UI transform because it will set the rectangle size... should fix this probably
  option->AddComponent<UIRectangleRenderComponent>();
  option->GetComponent<UIRectangleRenderComponent>()->isFilled = false;

  option->GetComponent<TextRenderer>()->SetFont(ResourceManager::Get().GetFontWriter("fonts\\Eurostile.ttf", 36));
  option->GetComponent<TextRenderer>()->SetText(label);

  option->GetComponent<MenuItem>()->location = position;
  option->GetComponent<MenuItem>()->callback = callback;
}
