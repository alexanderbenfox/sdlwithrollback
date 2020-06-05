#pragma once
#include "Entity.h"

class Camera;

class IScene
{
public:
  virtual void Init() = 0;
  virtual void Update(float deltaTime) = 0;
  virtual Camera* GetCamera() = 0;
  
};


/*class MainMenuScene : public IScene
{
public:
};

class CharacterSelectScene : public IScene
{
public:
};*/


class BattleScene : public IScene
{
public:
  virtual void Init() final;
  virtual void Update(float deltaTime) final;
  virtual Camera* GetCamera() final;

protected:
  std::shared_ptr<Entity> InitCharacter(Vector2<float> position);
  std::shared_ptr<Entity> _p1, _p2;
  std::shared_ptr<Entity> _camera;

};