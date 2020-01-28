#include "GameState/Player.h"

#include "Entity.h"
#include "Components/GameActor.h"
#include "Components/Animator.h"
#include "Components/Physics.h"
#include "Components/Collider.h"

float Player::GetCenterX() const
{
  return actor->transform.position.x + static_cast<float>(actor->GetComponent<RectColliderD>()->rect.Width() / 2.0);
}

std::ostream& operator<<(std::ostream& os, const Player& player)
{
  os << player.actor->transform;
  os << *player.actor->GetComponent<Animator>();
  os << *player.actor->GetComponent<GameActor>();
  os << *player.actor->GetComponent<Physics>();
  os << *player.actor->GetComponent<RectColliderD>();
  return os;
}

std::istream& operator>>(std::istream& is, Player& player)
{
  is >> player.actor->transform;
  is >> *player.actor->GetComponent<Animator>();
  is >> *player.actor->GetComponent<GameActor>();
  is >> *player.actor->GetComponent<Physics>();
  is >> *player.actor->GetComponent<RectColliderD>();
  return is;
}

#ifdef _WIN32
#endif