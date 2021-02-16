#include "Core/Prefab/MenuButtonArray.h"

#include "Components/Transform.h"
#include "Components/UIComponents.h"
#include "Components/RenderComponent.h"

#include "Systems/MenuSystem.h"
#include "Systems/DestroyEntitiesSystem.h"

#include "Managers/GameManagement.h"

MenuButtonArray::MenuButtonArray(int columns, int rows, float margin) : _size(columns, rows)
{
  _cellSize = Vector2<float>((float)m_nativeWidth / (float)_size.x, (float)m_nativeHeight / (float)_size.y);
  _margin = Vector2<float>(_cellSize.x * margin, _cellSize.y * margin);

  _btnIDs = std::vector<EntityID>(columns * rows);
}
  
void MenuButtonArray::CreateMenuOption(const char* label, std::function<void(SDL_Event)> callback, Vector2<int> position, bool triggerRawInput)
{
  auto option = GameManager::Get().CreateEntity<UITransform, RenderProperties, MenuItem, DestroyOnSceneEnd>();
  option->GetComponent<UITransform>()->anchor = UIAnchor::TL;

  // right now just make the menu cover the entire screen
  Vector2<float> optPosition(_cellSize.x * position.x + _margin.x, _cellSize.y * position.y + _margin.y);
  option->GetComponent<UITransform>()->position = optPosition;

  Rect<float> optRect(optPosition.x, optPosition.y, optPosition.x + _cellSize.x - 2.0f * _margin.x, optPosition.y + _cellSize.y - 2.0f * _margin.y);
  option->GetComponent<UITransform>()->rect = optRect;

  // add after setting the UI transform because it will set the rectangle size... should fix this probably
  //option->AddComponent<UIRectangleRenderComponent>();
  //option->GetComponent<UIRectangleRenderComponent>()->isFilled = false;
  option->AddComponent<UIBoxSpriteRenderComponent>();
  option->GetComponent<UIBoxSpriteRenderComponent>()->Init(ResourceManager::Get().GetAsset<GLTexture>("spritesheets/txtbox.png"), 3, 3, 0, 2, 6, 8, 3, 1, 5, 7);

  option->GetComponent<MenuItem>()->location = position;
  option->GetComponent<MenuItem>()->callback = callback;
  option->GetComponent<MenuItem>()->callbackOnPressDown = triggerRawInput;

  // create label text as sep entity so it can be centered
  auto labelText = GameManager::Get().CreateEntity<UITransform, TextRenderer, RenderProperties, DestroyOnSceneEnd>();
  labelText->GetComponent<UITransform>()->anchor = UIAnchor::Center;
  labelText->GetComponent<UITransform>()->parent = option->GetComponent<UITransform>();

  labelText->GetComponent<UITransform>()->rect = Rect<float>(0, 0, 0, 0);
  labelText->GetComponent<UITransform>()->position = Vector2<float>::Zero;

  labelText->GetComponent<TextRenderer>()->SetFont(ResourceManager::Get().GetFontWriter("fonts\\Eurostile.ttf", 36));
  labelText->GetComponent<TextRenderer>()->SetText(label, TextAlignment::Centered, _cellSize.x - 2.0f * _margin.x);

  // set id so things can be changed later
  int idx = position.y * _size.x + position.x;
  _btnIDs[idx] = labelText->GetID();
}

void MenuButtonArray::SetLabelText(int column, int row, const char* label) const
{
  int idx = row * _size.x + column;
  ComponentArray<TextRenderer>::Get().GetComponent(_btnIDs[idx]).SetText(label, TextAlignment::Centered, _cellSize.x - 2.0f * _margin.x);
}
