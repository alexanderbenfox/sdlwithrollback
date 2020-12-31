// 2DEngine.cpp : Defines the entry point for the console application.
//
#include "Managers/GameManagement.h"
#include "Managers/ResourceManager.h"

#include "Core/Utility/Profiler.h"

#include <iostream>

#ifdef _WIN32
#undef main
#endif

int main(int argc, char* args[])
{
  
  std::cout << "Initializing resource manager...";
  PROFILE_BEGIN_SESSION("InitializeResourceManager", "../profiling_data/Init.json");
  ResourceManager::Get().Initialize();
  PROFILE_END_SESSION();
  std::cout << "Success.\n";

  std::cout << "Initializing game manager...";
  PROFILE_BEGIN_SESSION("InitializeGameManager", "../profiling_data/Init.json");
  GameManager::Get().Initialize();
  PROFILE_END_SESSION();
  std::cout << "Success.\n";

  std::cout << "Beginning game loop...\n";
  PROFILE_BEGIN_SESSION("GameLoop", "../profiling_data/Runtime.json");
  GameManager::Get().BeginGameLoop();
  PROFILE_END_SESSION();
  std::cout << "Ending game loop... ending game.\n";

  PROFILE_BEGIN_SESSION("DestroyManagers", "../profiling_data/Destroy.json");
  ResourceManager::Get().Destroy();
  GameManager::Get().Destroy();
  PROFILE_END_SESSION();

  return 0;
}
