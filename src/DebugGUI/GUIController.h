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



  int AddImguiWindowFunction(const std::string& window, const std::string& category, std::function<void()> function);
  void RemoveImguiWindowFunction(const std::string& window, const std::string& category, int index);

private:
  GUIController() = default;
  GUIController(const GUIController&) = delete;
  GUIController operator=(GUIController&) = delete;

  SDL_Window* _window = nullptr;
  SDL_GLContext _glContext = nullptr;

  bool _ownsWindow = false;

  bool _drawComponentDebug = true;

  struct WindowFn
  {
    std::unordered_map<int, std::function<void()>> fns;
    int fnCount = 0;
  };
  typedef std::unordered_map<std::string, WindowFn> WindowGrouping;

  std::unordered_map<std::string, WindowGrouping> _imguiWindows;
};

struct DropDown
{
  static void Show(const char* currentItem, const char* items[], int nItems, std::function<void(const std::string&)> callback);
};
