#pragma once
#include <nanogui/screen.h>

class SimpleDebugWindow : public nanogui::Screen
{
public:
  SimpleDebugWindow(int w, int h, const char* name);

  virtual void draw(NVGcontext* ctx)
  {
    Screen::draw(ctx);
  }

  virtual void drawContents()
  {
    Screen::drawContents();
  }

private:
};