#pragma once
#include "Input.h"
#include "Components/IComponent.h"

class Entity;
class ICommand;

struct Player
{
  Player(std::shared_ptr<Entity> actor) : actor(actor) {}
  //! Currently controller object
  std::shared_ptr<Entity> actor;
  //!
  virtual float GetCenterX() const;

  friend std::ostream& operator<<(std::ostream& os, const Player& player);
  friend std::istream& operator>>(std::istream& is, Player& player);
};

struct LocalPlayer : public Player
{
  LocalPlayer(IInputHandler<SDL_Event>* input, std::shared_ptr<Entity> actor) : input(std::unique_ptr<IInputHandler<SDL_Event>>(input)), Player(actor) {}
  //! Player input listener
  std::unique_ptr<IInputHandler<SDL_Event>> input;
 
};

#ifdef _WIN32
struct NetworkPlayer : public Player
{
  NetworkPlayer(IInputHandler<GGPOInput>* input, std::shared_ptr<Entity> actor) : input(std::unique_ptr<IInputHandler<GGPOInput>>(input)), Player(actor) {}
  //! Player input listener
  std::unique_ptr<IInputHandler<GGPOInput>> input;
};
#endif