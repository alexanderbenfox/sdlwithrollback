#pragma once
#include <Windows.h>
#include <gl/glew.h>
#include <gl/GL.h>

#include <nanogui/opengl.h>
#include <nanogui/glutil.h>

#include <thread>

#include "DebugGUI/SimpleDebugWindow.h"

// nano gui singleton
class NanoGUI
{
public:
  static NanoGUI& Get()
  {
    static NanoGUI instance;
    return instance;
  }

  void BeginGUIThread(std::thread& guiThread);

private:
  NanoGUI() = default;
  NanoGUI(const NanoGUI&) = delete;
  NanoGUI operator=(NanoGUI&) = delete;

  bool InitGUIWindows();

  std::map<std::string, nanogui::ref<SimpleDebugWindow>> _windows;

};
