#pragma once
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
  void MainLoop(SDL_Event& event);
  void RenderFrame();
  void CleanUp();

  int AddImguiWindowFunction(const std::string& group, std::function<void()>& function);
  void RemoveImguiWindowFunction(const std::string& group, int index);

  void RenderToMainWindow(SDL_Renderer* renderer);

private:
  GUIController() = default;
  GUIController(const GUIController&) = delete;
  GUIController operator=(GUIController&) = delete;

  SDL_Window* _window = nullptr;
  SDL_GLContext _glContext = nullptr;

  GLTexture* _mainWindowTexture;

  std::unordered_map<std::string, std::vector<std::function<void()>>> _imguiWindowGroups;

};
