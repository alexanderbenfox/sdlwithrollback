// 2DEngine.cpp : Defines the entry point for the console application.
//
#include "GameManagement.h"

#include <iostream>

#ifdef _WIN32
#undef main
#endif

int main(int argc, char* args[])
{
  std::cout << "Initializing resource manager...";
  ResourceManager::Get().Initialize();
  std::cout << "Success.\n";

  std::cout << "Initializing game manager...";
  GameManager::Get().Initialize();
  std::cout << "Success.\n";

  std::cout << "Beginning game loop...\n";
  GameManager::Get().BeginGameLoop();
  std::cout << "Ending game loop... ending game.\n";

  ResourceManager::Get().Destroy();
  GameManager::Get().Destroy();

  return 0;
}
