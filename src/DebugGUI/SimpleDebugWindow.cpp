#include "DebugGUI/SimpleDebugWindow.h"
#include <nanogui/window.h>
#include <nanogui/layout.h>

SimpleDebugWindow::SimpleDebugWindow(int w, int h, const char* name) : nanogui::Screen(nanogui::Vector2i(w, h), name)
{
  using namespace nanogui;

  Window* window = new Window(this, name);
  window->setPosition(Vector2i(0, 0));
  window->setLayout(new GroupLayout());
}