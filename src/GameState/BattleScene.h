#pragma once
#include "GameState/Scene.h"

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
  std::shared_ptr<Entity> _p1UIAnchor, _p2UIAnchor;

};

