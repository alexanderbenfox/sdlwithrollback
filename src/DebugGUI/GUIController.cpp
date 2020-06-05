#include "DebugGUI/GLTexture.h"
#include "DebugGUI/GUIController.h"
#include "../imgui/imgui.h"
#include <stdio.h>

#include "GameManagement.h"

GUIController::~GUIController()
{
  if (_mainWindowTexture)
    delete _mainWindowTexture;
}

bool GUIController::InitSDLWindow()
{
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

  _mainWindowTexture = new GLTexture;
  _mainWindowTexture->CreateEmpty(m_nativeWidth, m_nativeHeight);

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

  // render the "main window"
  {
    ImGui::Begin("Game Window");
    ImGui::Image((void*)(intptr_t)_mainWindowTexture->texture(), ImVec2(m_nativeWidth, m_nativeHeight));
    ImGui::End();
  }

  for (auto& group : _imguiWindowGroups)
  {
    ImGui::Begin(group.first.c_str());
    for (auto& func : group.second)
    {
      func();
    }
    ImGui::End();
  }
}

void GUIController::CleanUp()
{
  // Cleanup
  ImGui_ImplOpenGL2_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
}


int GUIController::AddImguiWindowFunction(const std::string& group, std::function<void()>& function)
{
  auto it = _imguiWindowGroups.find(group);
  if (it == _imguiWindowGroups.end())
  {
    _imguiWindowGroups.emplace(group, std::vector<std::function<void()>>());
  }
  _imguiWindowGroups[group].push_back(function);

  // return the index of the new item
  return _imguiWindowGroups[group].size() - 1;
}

void GUIController::RemoveImguiWindowFunction(const std::string& group, int index)
{
  auto it = _imguiWindowGroups.find(group);
  if (it != _imguiWindowGroups.end())
  {
    it->second.erase(it->second.begin() + index);
  }
}

void GUIController::RenderToMainWindow(SDL_Renderer* renderer)
{
  SDL_Rect screenRect{ 0, 0, m_nativeWidth, m_nativeHeight };
  unsigned char buffer[m_nativeWidth * m_nativeHeight * 4];
  SDL_RenderReadPixels(renderer, &screenRect, GameManager::Get().GetWindowFormat(), buffer, 4 * m_nativeWidth);
  _mainWindowTexture->Update(buffer);
}

void GUIController::RenderFrame()
{
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  ImGuiIO& io = ImGui::GetIO();

  // Rendering
  ImGui::Render();
  glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
  glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
  glClear(GL_COLOR_BUFFER_BIT);
  //glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context where shaders may be bound
  ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
  SDL_GL_SwapWindow(_window);
}

