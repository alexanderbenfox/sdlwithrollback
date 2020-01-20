#include "GameState/Player.h"

#include "Command.h"
#include "Entity.h"
#include "Components/GameActor.h"
#include "Components/Sprite.h"
#include "Components/Physics.h"
#include "Components/Collider.h"

void LocalPlayer::ExecuteInput(ICommand* command)
{
  if (command)
    command->Execute(actor->GetComponent<GameActor>().get());
}

float LocalPlayer::GetCenterX()
{
  return actor->transform.position.x + static_cast<float>(actor->GetComponent<RectColliderD>()->rect.Width() / 2.0);
}

std::ostream& operator<<(std::ostream& os, const LocalPlayer& player)
{
  os << player.actor->transform;
  os << *player.actor->GetComponent<Animator>();
  os << *player.actor->GetComponent<GameActor>();
  os << *player.actor->GetComponent<Physics>();
  os << *player.actor->GetComponent<RectColliderD>();
  return os;
}

std::istream& operator>>(std::istream& is, LocalPlayer& player)
{
  is >> player.actor->transform;
  is >> *player.actor->GetComponent<Animator>();
  is >> *player.actor->GetComponent<GameActor>();
  is >> *player.actor->GetComponent<Physics>();
  is >> *player.actor->GetComponent<RectColliderD>();
  return is;
}

#ifdef _WIN32
void NetworkPlayer::ExecuteInput(ICommand* command)
{
  if (command)
    command->Execute(actor->GetComponent<GameActor>().get());
}

std::ostream& operator<<(std::ostream& os, const NetworkPlayer& player)
{
  os << player.actor->transform;
  os << *player.actor->GetComponent<Animator>();
  os << *player.actor->GetComponent<GameActor>();
  os << *player.actor->GetComponent<Physics>();
  os << *player.actor->GetComponent<RectColliderD>();
  return os;
}

std::istream& operator>>(std::istream& is, NetworkPlayer& player)
{
  is >> player.actor->transform;
  is >> *player.actor->GetComponent<Animator>();
  is >> *player.actor->GetComponent<GameActor>();
  is >> *player.actor->GetComponent<Physics>();
  is >> *player.actor->GetComponent<RectColliderD>();
  return is;
}
#endif
