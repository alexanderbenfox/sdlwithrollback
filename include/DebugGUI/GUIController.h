#pragma once
#include "../imgui/impl/imgui_impl_sdl.h"
#include "../imgui/impl/imgui_impl_opengl2.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <thread>
#include <vector>
#include <functional>

// nano gui singleton
class GUIController
{
public:
  static GUIController& Get()
  {
    static GUIController instance;
    return instance;
  }

  bool InitSDLWindow();
  bool InitImGUI();
  void MainLoop(SDL_Event& event);
  void RenderFrame();
  void CleanUp();

  void AddImguiWindowFunction(std::function<void()>& function)
  {
    _imguiWindowFunctions.push_back(function);
  }

private:
  GUIController() = default;
  GUIController(const GUIController&) = delete;
  GUIController operator=(GUIController&) = delete;

  SDL_Window* _window;
  SDL_GLContext _glContext;
  bool _done;

  std::vector<std::function<void()>> _imguiWindowFunctions;

};
