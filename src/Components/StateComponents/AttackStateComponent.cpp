#include "Components/StateComponents/AttackStateComponent.h"

AttackStateComponent::AttackStateComponent() :
  lastFrame(-1), IComponent()
{
}

void AttackStateComponent::Serialize(std::ostream& os) const
{
  Serializer<std::string>::Serialize(os, attackAnimation);
  Serializer<int>::Serialize(os, lastFrame);

  //serialize event list
  //write in n entries in set
  int types = (int)inProgressEventTypes.size();
  Serializer<int>::Serialize(os, types);
  for (const AnimationEvent::Type& type : inProgressEventTypes)
  {
    Serializer<AnimationEvent::Type>::Serialize(os, type);
  }
}

void AttackStateComponent::Deserialize(std::istream& is)
{
  Serializer<std::string>::Deserialize(is, attackAnimation);
  Serializer<int>::Deserialize(is, lastFrame);

  int nTypes = 0;
  Serializer<int>::Deserialize(is, nTypes);
  for (int i = 0; i < nTypes; i++)
  {
    AnimationEvent::Type type;
    Serializer<AnimationEvent::Type>::Deserialize(is, type);
    inProgressEventTypes.insert(type);
  }
}

std::string AttackStateComponent::Log()
{
  std::stringstream ss;
  ss << "Attack State Component: \n";
  ss << "\tAttack animation: " << attackAnimation << "\n";
  ss << "\tLast frame: " << lastFrame << "\n";
  return ss.str();
}
