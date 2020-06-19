#pragma once
#include "Entity.h"

class Camera;

class IScene
{
public:
  virtual ~IScene();
  virtual void Init(std::shared_ptr<Entity> p1, std::shared_ptr<Entity> p2) = 0;
  virtual void Update(float deltaTime) = 0;
  virtual Camera* GetCamera();

protected:
  std::shared_ptr<Entity> _camera;
  
};

enum class SceneType
{
  START, CSELECT, BATTLE, RESULTS
};

struct SceneHelper
{
  static IScene* CreateScene(SceneType type);
};

class StartScene : public IScene
{
public:
  virtual ~StartScene();
  virtual void Init(std::shared_ptr<Entity> p1, std::shared_ptr<Entity> p2) final;
  virtual void Update(float deltaTime) final;

protected:
  std::shared_ptr<Entity> _p1, _p2;
  std::shared_ptr<Entity> _renderedText;

};

class CharacterSelectScene : public IScene
{
public:
  virtual ~CharacterSelectScene();
  virtual void Init(std::shared_ptr<Entity> p1, std::shared_ptr<Entity> p2) final;
  virtual void Update(float deltaTime) final;
  
protected:
  std::shared_ptr<Entity> _p1, _p2;
  std::shared_ptr<Entity> _portrait;

};

class BattleScene : public IScene
{
public:
  virtual ~BattleScene();
  virtual void Init(std::shared_ptr<Entity> p1, std::shared_ptr<Entity> p2) final;
  virtual void Update(float deltaTime) final;

protected:
  static void InitCharacter(Vector2<float> position, std::shared_ptr<Entity> player);

  std::shared_ptr<Entity> _p1, _p2;
  std::shared_ptr<Entity> _borders[3];

};

class ResultsScene : public IScene
{
public:
  virtual ~ResultsScene();
  virtual void Init(std::shared_ptr<Entity> p1, std::shared_ptr<Entity> p2) final;
  virtual void Update(float deltaTime) final;

protected:
  std::shared_ptr<Entity> _p1, _p2;
  std::shared_ptr<Entity> _resultText;

};
