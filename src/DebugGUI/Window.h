#pragma once
#include "../imgui/imgui.h"

class GUIWindow
{
public:
  GUIWindow() = default;
  GUIWindow(const char* windowName) : _windowName(windowName) {}
  virtual ~GUIWindow() = default;

  int AddFn(const char* name, std::function<void()> fn)
  {
    for (int i = 0; i < nWindowSections; i++)
    {
      if (!_windowSections[i])
      {
        _windowSectionNames[i] = name;
        _windowSections[i] = fn;
        return i;
      }
        
    }
    return -1;
  }

  bool RemoveFn(int id)
  {
    if (id >= nWindowSections || id < 0)
      return false;

    _windowSectionNames[id] = "";
    _windowSections[id] = nullptr;
  }

  void OpenWindow()
  {
    if (!_windowDisplayed)
    {
      _windowDisplayed = true;
    }
  }

  void CloseWindow()
  {
    _windowDisplayed = false;
  }

  bool BeingViewed() const { return _windowDisplayed; }

  void DisplayWindow()
  {
    if (_windowDisplayed)
    {
      ImGui::Begin(_windowName.c_str());
      for (int i = 0; i < nWindowSections; i++)
      {
        // if no subsection function defined, skip
        if (!_windowSections[i])
          continue;

        ImGui::Text("%s", _windowSectionNames[i].c_str());
        _windowSections[i]();
      }
      if (ImGui::Button("Close")) { CloseWindow(); }
      ImGui::End();
    }
  }

private:
  std::string _windowName;
  //! bool for if the window is currently displayed
  bool _windowDisplayed = false;

  static constexpr size_t nWindowSections = 10;
  std::string _windowSectionNames[nWindowSections];
  std::function<void()> _windowSections[nWindowSections];

};