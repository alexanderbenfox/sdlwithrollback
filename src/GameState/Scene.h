#pragma once
#include "Core/ECS/Entity.h"
#include "Components/Actors/CutsceneActor.h"

class Camera;

class IScene
{
public:
  virtual ~IScene();
  virtual void Init(std::shared_ptr<Entity> p1, std::shared_ptr<Entity> p2) = 0;
  virtual void Update(float deltaTime) = 0;
  
};

enum class SceneType
{
  START, CSELECT, BATTLE, POSTMATCH, RESULTS
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
  std::shared_ptr<Entity> _uiCamera;

};

class CharacterSelectScene : public IScene
{
public:
  virtual ~CharacterSelectScene();
  virtual void Init(std::shared_ptr<Entity> p1, std::shared_ptr<Entity> p2) final;
  virtual void Update(float deltaTime) final;
  
protected:
  std::shared_ptr<Entity> _p1, _p2;
  std::shared_ptr<Entity> _uiCamera;

};

class BattleScene : public IScene
{
public:
  virtual ~BattleScene();
  virtual void Init(std::shared_ptr<Entity> p1, std::shared_ptr<Entity> p2) final;
  virtual void Update(float deltaTime) final;

  struct StageBorders
  {
    std::shared_ptr<Entity> borders[3];
    Rect<float> clamp;
  };

  static StageBorders CreateStageBorders(const Rect<float>& stageRect, int screenWidth, int screenHeight);

protected:
  void InitCharacter(Vector2<float> position, std::shared_ptr<Entity> player, bool isPlayer1);

  std::shared_ptr<Entity> _p1, _p2;
  // entities to be destroyed after this scene ends
  std::shared_ptr<Entity> _borders[3];
  std::shared_ptr<Entity> _p1UIAnchor, _p2UIAnchor;
  std::vector<std::shared_ptr<Entity>> _uiEntities;

  std::shared_ptr<Entity> _uiCamera, _camera;

};

class PostMatchScene : public IScene
{
public:
  PostMatchScene() : winnerAction1(2), winnerAction2("Win"),
    loserAction1("KO"), loserAction2(3), IScene() {}
  virtual ~PostMatchScene();
  virtual void Init(std::shared_ptr<Entity> p1, std::shared_ptr<Entity> p2) final;
  virtual void Update(float deltaTime) final;

protected:
  std::shared_ptr<Entity> _p1, _p2;
  std::shared_ptr<Entity> _borders[3];


  Wait winnerAction1;
  PlayAnimation winnerAction2;
  PlayAnimation loserAction1;
  Wait loserAction2;

  CutsceneAction* _winnerActions[2] = {&winnerAction1, &winnerAction2};
  CutsceneAction* _loserActions[2] = {&loserAction1, &loserAction2};

  std::shared_ptr<Entity> _uiCamera, _camera;
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
  std::shared_ptr<Entity> _uiCamera;

};
