#include "GameState/Player.h"

#include "Entity.h"
#include "Components/GameActor.h"
#include "Components/Animator.h"
#include "Components/Rigidbody.h"
#include "Components/Collider.h"

float Player::GetCenterX() const
{
  return actor->GetComponent<Transform>()->position.x + static_cast<float>(actor->GetComponent<RectColliderD>()->rect.Width() / 2.0);
}

std::ostream& operator<<(std::ostream& os, const Player& player)
{
  os << *player.actor->GetComponent<Transform>();
  os << *player.actor->GetComponent<Animator>();
  os << *player.actor->GetComponent<GameActor>();
  os << *player.actor->GetComponent<Rigidbody>();
  os << *player.actor->GetComponent<RectColliderD>();
  return os;
}

std::istream& operator>>(std::istream& is, Player& player)
{
  is >> *player.actor->GetComponent<Transform>();
  is >> *player.actor->GetComponent<Animator>();
  is >> *player.actor->GetComponent<GameActor>();
  is >> *player.actor->GetComponent<Rigidbody>();
  is >> *player.actor->GetComponent<RectColliderD>();
  return is;
}

#ifdef _WIN32
#endif
