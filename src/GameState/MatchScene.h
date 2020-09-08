#pragma once
#include "GameState/Scene.h"

//! fades in the text "ROUND X"... "FIGHT"
class PreMatchScene : public ISubScene
{
public:
  PreMatchScene(MatchMetaComponent& matchData);
  virtual ~PreMatchScene();
  virtual void Init(std::shared_ptr<Entity> p1, std::shared_ptr<Entity> p2) final;
  virtual void Update(float deltaTime) final;

protected:
  std::shared_ptr<Entity> _p1, _p2;
  std::shared_ptr<Entity> _roundText, _fightText;

  Wait _waitForEntranceComplete;

  AlphaFader _fadeAction1, _fadeAction2;
  Wait _wait1, _wait2;
  CutsceneAction* _fadeActionSet1[3] = { &_waitForEntranceComplete, &_fadeAction1, &_wait1 };
  CutsceneAction* _fadeActionSet2[2] = { &_wait2, &_fadeAction2 };

  WaitForTime _pauseP1, _pauseP2;
  PlayAnimation _entranceAction, _idle;
  CutsceneAction* _PCEntranceActionSet1[3] = { &_pauseP1, &_entranceAction, &_idle };
  CutsceneAction* _PCEntranceActionSet2[3] = { &_pauseP2, &_entranceAction, &_idle };

};

class PostMatchScene : public ISubScene
{
public:
  PostMatchScene(MatchMetaComponent& matchData);
  virtual ~PostMatchScene();
  virtual void Init(std::shared_ptr<Entity> p1, std::shared_ptr<Entity> p2) final;
  virtual void Update(float deltaTime) final;

protected:
  std::shared_ptr<Entity> _p1, _p2;
  std::shared_ptr<Entity> _koText;

  WaitForTime KODisplay, poseHold;
  AlphaFader fadeOutKO;

  Wait winnerAction1;
  PlayAnimation winnerAction2;
  PlayAnimation loserAction1;
  Wait loserAction2;

  Wait waitForText;

  CutsceneAction* _textActions[2] = { &KODisplay, &fadeOutKO };
  CutsceneAction* _winnerActions[4] = { &waitForText, &winnerAction1, &winnerAction2, &poseHold };
  CutsceneAction* _loserActions[2] = { &loserAction1, &loserAction2 };
};

class MatchScene : public IScene
{
public:
  virtual ~MatchScene();
  virtual void Init(std::shared_ptr<Entity> p1, std::shared_ptr<Entity> p2) final;
  virtual void Update(float deltaTime) final;
  virtual void AdvanceScene() final;

  BattleType _battleType = BattleType::Training;

protected:
  std::shared_ptr<ISubScene> _subScene;

  std::shared_ptr<Entity> _matchData;
  std::shared_ptr<Entity> _stageBorders[3];
  std::shared_ptr<Entity> _p1, _p2;
  std::shared_ptr<Entity> _uiCamera, _camera;

  MatchStage _currStage = MatchStage::BATTLE;
};
