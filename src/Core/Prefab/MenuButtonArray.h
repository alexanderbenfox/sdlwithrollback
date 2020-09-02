#pragma once
#include "Core/Math/Vector2.h"

#include <functional>

class MenuButtonArray
{
public:
  MenuButtonArray(int columns, int rows, float margin);
  void CreateMenuOption(const char* label, std::function<void()> callback, Vector2<int> position);

private:
  Vector2<int> _size;
  Vector2<float> _cellSize;
  Vector2<float> _margin;

};