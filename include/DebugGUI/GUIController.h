#pragma once
#include "Rendering/GLTexture.h"
#include "../imgui/impl/imgui_impl_sdl.h"
#include "../imgui/impl/imgui_impl_opengl2.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <string>
#include <thread>
#include <vector>
#include <functional>
#include <unordered_map>

class GLTexture;

// nano gui singleton
class GUIController
{
public:
  static GUIController& Get()
  {
    static GUIController instance;
    return instance;
  }

  ~GUIController();

  bool InitSDLWindow();
  bool InitImGUI();
  bool InitImGUI(SDL_Window* existingWindow, SDL_GLContext existingContext);

  void MainLoop(SDL_Event& event);
  void RenderFrame();
  void CleanUp();

  int AddImguiWindowFunction(const std::string& group, std::function<void()>& function);
  void RemoveImguiWindowFunction(const std::string& group, int index);

private:
  GUIController() = default;
  GUIController(const GUIController&) = delete;
  GUIController operator=(GUIController&) = delete;

  SDL_Window* _window = nullptr;
  SDL_GLContext _glContext = nullptr;

  bool _ownsWindow = false;

  bool _drawComponentDebug = true;

  std::unordered_map<std::string, std::vector<std::function<void()>>> _imguiWindowGroups;

};
