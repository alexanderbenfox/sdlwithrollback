#pragma once
#include <Windows.h>
#include <gl/GL.h>
#include <nanogui/opengl.h>
#include <nanogui/glutil.h>

// nano gui singleton
class NanoGUI
{
public:
  static NanoGUI& Get()
  {
    static NanoGUI instance;
    return instance;
  }

private:
  NanoGUI() { Init(); }
  NanoGUI(const NanoGUI&) = delete;
  NanoGUI operator=(NanoGUI&) = delete;

  bool Init();

};
