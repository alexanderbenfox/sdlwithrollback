#pragma once
#include "../imgui/impl/imgui_impl_sdl.h"
#include "../imgui/impl/imgui_impl_opengl2.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <thread>
#include <vector>
#include <functional>
#include <unordered_map>

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

  void AddImguiWindowFunction(const std::string& group, std::function<void()>& function)
  {
    auto it = _imguiWindowGroups.find(group);
    if (it == _imguiWindowGroups.end())
    {
      _imguiWindowGroups.emplace(group, std::vector<std::function<void()>>());
    }
    _imguiWindowGroups[group].push_back(function);
  }

private:
  GUIController() = default;
  GUIController(const GUIController&) = delete;
  GUIController operator=(GUIController&) = delete;

  SDL_Window* _window;
  SDL_GLContext _glContext;
  bool _done;

  std::unordered_map<std::string , std::vector<std::function<void()>>> _imguiWindowGroups;

};
