#pragma once
#include "Core/Math/Vector2.h"
#include "../SDL2/SDL.h"

#include <functional>

class MenuButtonArray
{
public:
  MenuButtonArray(int columns, int rows, float margin);
  void CreateMenuOption(const char* label, std::function<void(SDL_Event)> callback, Vector2<int> position, bool triggerRawInput = false);
  void SetLabelText(int column, int row, const char* label) const;

private:
  Vector2<int> _size;
  Vector2<float> _cellSize;
  Vector2<float> _margin;
  //!
  std::vector<unsigned int> _btnIDs;

};