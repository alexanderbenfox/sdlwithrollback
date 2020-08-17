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

void GUIController::MainLoop(SDL_Event& event)
{
  ImGuiIO& io = ImGui::GetIO();
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  // Poll and handle events (inputs, window resize, etc.)
  ImGui_ImplSDL2_ProcessEvent(&event);

  // Start the Dear ImGui frame
  ImGui_ImplOpenGL2_NewFrame();
  ImGui_ImplSDL2_NewFrame(_window);
  ImGui::NewFrame();

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
    ImGui::EndMainMenuBar();
  }

  if(showGUI)
  {
    for (auto& window : _imguiWindows)
    {
      ImGui::Begin(window.first.c_str());
      for(auto& category : window.second)
      {
        ImGui::Text(category.first.c_str());
        for (auto it : category.second.fns)
        {
          it.second();
        }
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
  auto windowsIt = _imguiWindows.find(window);
  if (windowsIt == _imguiWindows.end())
  {
    _imguiWindows.emplace(window, WindowGrouping());
  }
  auto categoryIt = _imguiWindows[window].find(category);
  if (categoryIt == _imguiWindows[window].end())
  {
    _imguiWindows[window].emplace(category, WindowFn());
  }

  // this number will eventually get very high... need to swap ids
  int debugID = _imguiWindows[window][category].fnCount;
  _imguiWindows[window][category].fns[debugID] = function;
  _imguiWindows[window][category].fnCount++;

  // return the index of the new item
  return debugID;
}

void GUIController::RemoveImguiWindowFunction(const std::string& window, const std::string& category, int index)
{
  auto windowsIt = _imguiWindows.find(window);
  if (windowsIt != _imguiWindows.end())
  {
    auto it = _imguiWindows[window].find(category);
    if (it != _imguiWindows[window].end())
    {
      it->second.fns.erase(index);
      if (it->second.fns.empty())
      {
        //reset fn counter as a silly work around for now
        it->second.fnCount = 0;
      }
    }
  }
}

void GUIController::RenderFrame()
{
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
