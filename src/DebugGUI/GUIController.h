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

#include "Window.h"

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

  void UpdateLogic(const SDL_Event& event);
  void MainLoop();
  void RenderFrame();
  void CleanUp();


  void CreatePopup(std::function<void()> display, std::function<void()> onClose)
  {
    _popup.display = display;
    _popup.onClose = onClose;
    _popup.setPopupSize = false;
  }
  void CreatePopup(std::function<void()> display, std::function<void()> onClose, float xSize, float ySize)
  {
    _popup.display = display;
    _popup.onClose = onClose;
    _popup.setPopupSize = true;
    _popup.popupSize = ImVec2(xSize, ySize);
  }

  int AddImguiWindowFunction(const std::string& window, const std::string& category, std::function<void()> function);
  int AddImguiWindowFunction(const std::string& mainMenu, const std::string& window, const std::string& category, std::function<void()> function);
  void RemoveImguiWindowFunction(const std::string& window, int index);

  void AddMenuItem(const std::string& menuName, const std::string& itemName, std::function<void()> onPress);

  bool HasWindow(const std::string& window) const { return _imguiWindows.find(window) != _imguiWindows.end(); }
  GUIWindow& GetWindow(const std::string& window) { return _imguiWindows[window]; }

private:
  GUIController() = default;
  GUIController(const GUIController&) = delete;
  GUIController operator=(GUIController&) = delete;

  SDL_Window* _window = nullptr;
  SDL_GLContext _glContext = nullptr;

  bool _ownsWindow = false;
  bool _drawComponentDebug = true;
  bool _init = false;

  std::unordered_map<std::string, GUIWindow> _imguiWindows;
  std::unordered_map<std::string, std::vector<std::pair<std::string, std::function<void()>>>> _menuFunctions;

  struct Popup
  {
    bool setPopupSize;
    ImVec2 popupSize;
    std::function<void()> display;
    std::function<void()> onClose;
  };
  Popup _popup;
};

struct DropDown
{
  static void DisplayList(const std::vector<std::string>& list, std::string& selection);
  static void DisplayList(const std::vector<std::string>& list, std::string& selection, std::function<void()> onSelect);
  static void Show(const char* currentItem, const char* items[], int nItems, std::function<void(const std::string&)> callback);
};
