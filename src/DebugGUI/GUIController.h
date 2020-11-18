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

  int GetPopupIndex(const std::string& label)
  {
    int replaceIndex = 0;
    bool replaceOnType = false;
    for (int i = 0; i < _activePopups; i++)
    {
      if (_popup[i].label == label)
      {
        replaceIndex = i;
        replaceOnType = true;
        break;
      }
    }

    if (replaceOnType || _activePopups == _maxPopups)
    {
      if (_popup[replaceIndex].display != nullptr)
        ClosePopup(replaceIndex);
    }

    if (_activePopups < _maxPopups)
      _activePopups++;
    return _activePopups - 1;
  }

  void CreatePopup(std::string label, std::function<void()> display, std::function<void()> onClose)
  {
    int idx = GetPopupIndex(label);
    _popup[idx].label = label;
    _popup[idx].display = display;
    _popup[idx].onClose = onClose;
    _popup[idx].setPopupSize = false;
  }
  void CreatePopup(std::string label, std::function<void()> display, std::function<void()> onClose, float xSize, float ySize)
  {
    int idx = GetPopupIndex(label);
    _popup[idx].label = label;
    _popup[idx].display = display;
    _popup[idx].onClose = onClose;
    _popup[idx].setPopupSize = true;
    _popup[idx].popupSize = ImVec2(xSize, ySize);
  }

  void ClosePopup(int windowIndex)
  {
    if (windowIndex > (_activePopups - 1))
      return;

    for (int i = windowIndex; i < (_activePopups - 1); i++)
    {
      _popup[i] = _popup[i + 1];
    }
    // "delete" last index
    _popup[_activePopups - 1].display = nullptr;
    _popup[_activePopups - 1].onClose = nullptr;
    _activePopups--;
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
    std::string label;
    bool setPopupSize;
    ImVec2 popupSize;
    std::function<void()> display;
    std::function<void()> onClose;
  };

  static constexpr int _maxPopups = 3;
  int _activePopups = 0;
  Popup _popup[_maxPopups];
};

struct DropDown
{
  static void DisplayList(const std::vector<std::string>& list, std::string& selection);
  static void DisplayList(const std::vector<std::string>& list, std::string& selection, std::function<void()> onSelect);
  static void Show(const char* currentItem, const char* items[], int nItems, std::function<void(const std::string&)> callback);

  static int activeDropDowns;
};
