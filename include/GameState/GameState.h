#pragma once
#include "Input.h"
#include <memory>

class Entity;
class Camera;

struct Player
{
  //! Player input listener
  std::unique_ptr<IInputHandler> input;
  //! Currently controller object
  std::shared_ptr<Entity> actor;
  //!
  void ExecuteInput(ICommand* command);

  friend std::ostream& operator<<(std::ostream& os, const Player& player);
  friend std::istream& operator>>(std::istream& is, Player& player);
};


//______________________________________________________________________________
//!
class IGameState
{
public:
  virtual ~IGameState() {}
  //virtual void LoadAssets() = 0;
  virtual void ProcessInputs(SDL_Event* localInput) = 0;

  virtual Camera* GetCamera() = 0;
};

class LocalMatch : public IGameState
{
public:
  LocalMatch(IInputHandler* p1, IInputHandler* p2) : _player1(CreatePlayer(p1, 0)), _player2(CreatePlayer(p2, 150))
  {
    CreateCamera();
  }

  virtual void ProcessInputs(SDL_Event* localInput) override;
  virtual Camera* GetCamera() override { return _mainCamera.get(); }


protected:
  //! Camera used by rendering pipeline to view the scene
  std::shared_ptr<Camera> _mainCamera;
  //! 
  Player _player1, _player2;

  Player CreatePlayer(IInputHandler* input, float xOffset);
  void CreateCamera();

};
