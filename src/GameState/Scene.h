#pragma once
#include "Core/ECS/Entity.h"
#include "Components/Actors/CutsceneActor.h"
#include "Components/MetaGameComponents.h"

// for ctrl set up scene... probably should be moved
#include "Core/Prefab/MenuButtonArray.h"

class Camera;

class IScene
{
public:
  virtual ~IScene() {}
  virtual void Init(std::shared_ptr<Entity> p1, std::shared_ptr<Entity> p2) = 0;
  virtual void Update(float deltaTime) = 0;
  virtual void AdvanceScene() {}
  
};

class ISubScene : public IScene
{
public:
  ISubScene(MatchMetaComponent& matchData) : _matchStatus(matchData), IScene() {}
  virtual ~ISubScene() {}

protected:
  //! match status when this scene was initialized
  MatchMetaComponent& _matchStatus;

};

enum class MainMenuOptions
{
  Main, Online, Versus, Solo, Options
};

enum class OnlineMenuOptions
{
  Ranked, Invite, Rankings
};

enum class SoloMenuOptions
{
  Arcade, VS_CPU, Career, Training, Lessons
};

enum class SceneType
{
  START, CTRLSETUP, BATTLEMODE, CSELECT, MATCH, RESULTS
};

// eventually use this for initiating different battle scene types
enum class BattleType
{
  Training,
  BestOf3,
  BestOf5
};

enum class MatchStage
{
  PREMATCH, BATTLE, POSTMATCH
};

const Vector2<float> cameraOrigin(m_nativeWidth / 2.0f, m_nativeHeight / 2.0f);

struct SceneHelper
{
  static IScene* CreateScene(SceneType type);
};

class BasicMenuScene : public IScene
{
public:
  virtual ~BasicMenuScene() = default;
  virtual void Update(float deltaTime) override;

};

class StartScene : public BasicMenuScene
{
public:
  virtual ~StartScene();
  virtual void Init(std::shared_ptr<Entity> p1, std::shared_ptr<Entity> p2) final;

protected:
  std::shared_ptr<Entity> _p1, _p2;
  std::shared_ptr<Entity> _uiCamera;

};

class CtrlSetupScene : public BasicMenuScene
{
public:
  CtrlSetupScene() : _menu(1, 9, 0.01f), BasicMenuScene() {}
  virtual ~CtrlSetupScene();
  virtual void Init(std::shared_ptr<Entity> p1, std::shared_ptr<Entity> p2) final;

protected:
  std::shared_ptr<Entity> _p1, _p2;
  std::shared_ptr<Entity> _uiCamera;
  MenuButtonArray _menu;

};

class BattleModeSelect : public BasicMenuScene
{
public:
  virtual ~BattleModeSelect();
  virtual void Init(std::shared_ptr<Entity> p1, std::shared_ptr<Entity> p2) final;
  
protected:
  std::shared_ptr<Entity> _p1, _p2;
  std::shared_ptr<Entity> _uiCamera;

};

class CharacterSelect : public BasicMenuScene
{
public:
  virtual ~CharacterSelect();
  virtual void Init(std::shared_ptr<Entity> p1, std::shared_ptr<Entity> p2) final;

protected:
  std::shared_ptr<Entity> _p1, _p2;
  std::shared_ptr<Entity> _headerLabel;
  bool _firstCharacter = false;
  bool _secondCharacter = false;

};

class ResultsScene : public BasicMenuScene
{
public:
  virtual ~ResultsScene();
  virtual void Init(std::shared_ptr<Entity> p1, std::shared_ptr<Entity> p2) final;

protected:
  std::shared_ptr<Entity> _p1, _p2;
  std::shared_ptr<Entity> _resultText;
  std::shared_ptr<Entity> _uiCamera;

};
