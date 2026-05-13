#include "DebugGUI/GUIController.h"
#include "../imgui/imgui.h"

#include <stdio.h>
#include "Managers/GameManagement.h"
#include "Rendering/RenderManager.h"

GUIController::~GUIController()
{
  _alive = false;
  _window = nullptr;
}

bool GUIController::InitImGUI()
{
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;
  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  // Setup Platform/Renderer bindings
  _window = RenderManager::Get().GetWindow();
  ImGui_ImplSDL2_InitForMetal(_window);
  ImGui_ImplBgfx_Init(VIEW_IMGUI);
  _ownsWindow = false;

  return true;
}

bool GUIController::InitImGUI(SDL_Window* existingWindow)
{
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  // Setup Platform/Renderer bindings
  ImGui_ImplSDL2_InitForMetal(existingWindow);
  ImGui_ImplBgfx_Init(VIEW_IMGUI);
  _window = existingWindow;

  _ownsWindow = false;

  return true;
}

void GUIController::UpdateLogic(const SDL_Event& event)
{
  // Poll and handle events (inputs, window resize, etc.)
  ImGui_ImplSDL2_ProcessEvent(&event);
}

void GUIController::MainLoop()
{
  // Start the Dear ImGui frame
  ImGui_ImplBgfx_NewFrame();
  ImGui_ImplSDL2_NewFrame(_window);
  ImGui::NewFrame();

  // inform that we can now render gui
  _init = true;

  static bool showGUI = true;
  if (ImGui::BeginMainMenuBar())
  {
    if (ImGui::BeginMenu("File"))
    {
      if (ImGui::MenuItem("Toggle DebugUI", ""))
      {
        showGUI = !showGUI;
      }
      if (ImGui::MenuItem("Toggle In Scene Debug", ""))
      {
        _drawComponentDebug = !_drawComponentDebug;
      }
      ImGui::EndMenu();
    }

    for (auto& menu : _menuFunctions)
    {
      if (ImGui::BeginMenu(menu.first.c_str()))
      {
        for (auto& function : menu.second)
        {
          if (ImGui::MenuItem(function.first.c_str(), ""))
          {
            function.second();
          }
        }
        ImGui::EndMenu();
      }
    }

    ImGui::EndMainMenuBar();
  }

  if(showGUI)
  {
    for (auto& window : _imguiWindows)
    {
      window.second.DisplayWindow();
    }

    std::pair<bool, int> closeOperation = { false, 0 };
    for (int i = 0; i < _activePopups; i++)
    {
      if (_popup[i].display && _popup[i].onClose)
      {
        if (_popup[i].setPopupSize)
          ImGui::SetNextWindowSize(_popup[i].popupSize);

        std::string label = "Popup: " + _popup[i].label;
        ImGui::Begin(label.c_str());
        _popup[i].display();
        if (ImGui::Button("Close")) { closeOperation = { true, i }; }
        ImGui::End();
      }
    }
    if (closeOperation.first)
    {
      _popup[closeOperation.second].onClose();
      ClosePopup(closeOperation.second);
    }

  }

  // reset active drop downs on end of main loop (clean up)
  DropDown::activeDropDowns = 0;
}

void GUIController::CleanUp()
{
  // Cleanup
  ImGui_ImplBgfx_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
}

int GUIController::AddImguiWindowFunction(const std::string& window, const std::string& category, std::function<void()> function)
{
  return AddImguiWindowFunction("Windows", window, category, function);
}

int GUIController::AddImguiWindowFunction(const std::string& mainMenu, const std::string& window, const std::string& category, std::function<void()> function)
{
  auto windowsIt = _imguiWindows.find(window);
  if (windowsIt == _imguiWindows.end())
  {
    _imguiWindows.emplace(window, window.c_str());
    AddMenuItem(mainMenu, window, [this, window]()
      {
        if (!_imguiWindows[window].BeingViewed())
          _imguiWindows[window].OpenWindow();
        else
          _imguiWindows[window].CloseWindow();
      });
  }

  return _imguiWindows[window].AddFn(category.c_str(), function);
}

void GUIController::RemoveImguiWindowFunction(const std::string& window, int index)
{
  auto windowsIt = _imguiWindows.find(window);
  if (windowsIt != _imguiWindows.end())
  {
    _imguiWindows[window].RemoveFn(index);
  }
}

void GUIController::AddMenuItem(const std::string& menuName, const std::string& itemName, std::function<void()> onPress)
{
  auto menu = _menuFunctions.find(menuName);
  if (menu == _menuFunctions.end())
  {
    _menuFunctions.insert(std::make_pair(menuName, std::vector<std::pair<std::string, std::function<void()>>>()));
  }
  _menuFunctions[menuName].push_back(std::make_pair(itemName, onPress));
}

void GUIController::RenderFrame()
{
  if (!_init)
    return;

  // draw the "in-game" debug information before drawing the imgui UI
  if (_drawComponentDebug)
  {
    GameManager::Get().DebugDraws();
  }

  // Rendering
  ImGui::Render();
  ImGui_ImplBgfx_RenderDrawData(ImGui::GetDrawData());
}

//! Init to 0
int DropDown::activeDropDowns = 0;

void DropDown::DisplayList(const std::vector<std::string>& list, std::string& selection)
{
  std::vector<const char*> valuesCStr;
  for (const auto& item : list)
  {
    valuesCStr.push_back(item.c_str());
  }
  DropDown::Show(selection.c_str(), valuesCStr.data(), static_cast<int>(valuesCStr.size()), [&selection](const std::string& selected) { selection = selected; });
}


void DropDown::DisplayList(const std::vector<std::string>& list, std::string& selection, std::function<void()> onSelect)
{
  std::vector<const char*> valuesCStr;
  for (const auto& item : list)
  {
    valuesCStr.push_back(item.c_str());
  }
  DropDown::Show(selection.c_str(), valuesCStr.data(), static_cast<int>(valuesCStr.size()), [&selection, onSelect](const std::string& selected) { selection = selected; onSelect(); });
}

void DropDown::Show(const char* currentItem, const char* items[], int nItems, std::function<void(const std::string&)> callback)
{
  std::string ddName = "##combo" + std::to_string(activeDropDowns++);
  if (ImGui::BeginCombo(ddName.c_str(), currentItem))
  {
    for (int n = 0; n < nItems; n++)
    {
      bool is_selected = (currentItem == items[n]);
      if (ImGui::Selectable(items[n], is_selected))
      {
        currentItem = items[n];
        std::string itemString(items[n]);
        callback(itemString);
      }
      if (is_selected)
        ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }
}
