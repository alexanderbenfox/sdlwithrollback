#pragma once
#include "Input.h"
#include "Components/IComponent.h"

class Entity;
class ICommand;

class PlayerData : public IComponent
{
public:
  //!
  PlayerData(std::shared_ptr<Entity> entity) : flipped(false), IComponent(entity) {}
  //!
  bool flipped;
};

struct LocalPlayer
{
  LocalPlayer(IInputHandler<SDL_Event>* input, std::shared_ptr<Entity> actor) : input(std::unique_ptr<IInputHandler<SDL_Event>>(input)), actor(actor) {}
  //! Player input listener
  std::unique_ptr<IInputHandler<SDL_Event>> input;
  //! Currently controller object
  std::shared_ptr<Entity> actor;
  //!
  void ExecuteInput(ICommand* command);
  //!
  float GetCenterX();

  friend std::ostream& operator<<(std::ostream& os, const LocalPlayer& player);
  friend std::istream& operator>>(std::istream& is, LocalPlayer& player);
};

#ifdef _WIN32
struct NetworkPlayer
{
  NetworkPlayer(IInputHandler<GGPOInput>* input, std::shared_ptr<Entity> actor) : input(std::unique_ptr<IInputHandler<GGPOInput>>(input)), actor(actor) {}
  //! Player input listener
  std::unique_ptr<IInputHandler<GGPOInput>> input;
  //! Currently controller object
  std::shared_ptr<Entity> actor;
  //!
  void ExecuteInput(ICommand* command);

  friend std::ostream& operator<<(std::ostream& os, const NetworkPlayer& player);
  friend std::istream& operator>>(std::istream& is, NetworkPlayer& player);
};
#endif