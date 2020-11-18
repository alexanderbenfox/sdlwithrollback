#include "DebugGUI/GUIController.h"
#include "../imgui/imgui.h"

#include <stdio.h>
#include "Managers/GameManagement.h"
#include "Rendering/RenderManager.h"

GUIController::~GUIController()
{
  if (_ownsWindow)
  {
    SDL_DestroyWindow(_window);
    SDL_GL_DeleteContext(_glContext);
  }

  _window = nullptr;
  _glContext = nullptr;
}

bool GUIController::InitSDLWindow()
{
  if (_ownsWindow)
  {
    SDL_DestroyWindow(_window);
    SDL_GL_DeleteContext(_glContext);

    _window = nullptr;
    _glContext = nullptr;
  }

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
  SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

  _window = SDL_CreateWindow("Dear ImGui SDL2+OpenGL example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
  _glContext = SDL_GL_CreateContext(_window);
  SDL_GL_MakeCurrent(_window, _glContext);

  // Enable vsync
  SDL_GL_SetSwapInterval(1); 
  _ownsWindow = true;

  return true;
}

bool GUIController::InitImGUI()
{
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  //ImGui::StyleColorsClassic();

  // Setup Platform/Renderer bindings
  ImGui_ImplSDL2_InitForOpenGL(_window, _glContext);
  ImGui_ImplOpenGL2_Init();
  return true;
}

bool GUIController::InitImGUI(SDL_Window* existingWindow, SDL_GLContext existingContext)
{
  if (_ownsWindow)
  {
    SDL_DestroyWindow(_window);
    SDL_GL_DeleteContext(_glContext);

    _window = nullptr;
    _glContext = nullptr;
  }

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  // Setup Platform/Renderer bindings
  ImGui_ImplSDL2_InitForOpenGL(existingWindow, existingContext);
  ImGui_ImplOpenGL2_Init();
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
  ImGuiIO& io = ImGui::GetIO();
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  // Start the Dear ImGui frame
  ImGui_ImplOpenGL2_NewFrame();
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

    if (_popup.display && _popup.onClose)
    {
      if (_popup.setPopupSize)
        ImGui::SetNextWindowSize(_popup.popupSize);

      ImGui::Begin("Popup");
      _popup.display();
      if (ImGui::Button("Close"))
      {
        _popup.onClose();
        _popup.display = nullptr;
        _popup.onClose = nullptr;
      }
      ImGui::End();
    }
  }
}

void GUIController::CleanUp()
{
  // Cleanup
  ImGui_ImplOpenGL2_Shutdown();
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

  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  ImGuiIO& io = ImGui::GetIO();

  // Rendering
  ImGui::Render();
  glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
  if (_ownsWindow)
  {
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
  }

  //glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context where shaders may be bound
  ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

  if(_ownsWindow)
    SDL_GL_SwapWindow(_window);
}


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
  if (ImGui::BeginCombo("##combo", currentItem))
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
