#include "ISystem.h"
#include "Components/AIComponent.h"

class UpdateAISystem : public IMultiSystem<SysComponents<AIComponent, Transform, StateComponent, TeamComponent>, SysComponents<Transform, StateComponent, TeamComponent>>
{
public:
  static void DoTick(float dt)
  {
    for (auto tuple : MainSystem::Tuples)
    {
      AIComponent* ai = std::get<AIComponent*>(tuple.second);
      Transform* transform = std::get<Transform*>(tuple.second);
      StateComponent* state = std::get<StateComponent*>(tuple.second);
      TeamComponent* team = std::get<TeamComponent*>(tuple.second);

      ai->UpdateMyState(transform, state);
      
      for(auto subTuple : SubSystem::Tuples)
      {
        Transform* otherTransform = std::get<Transform*>(tuple.second);
        StateComponent* otherState = std::get<StateComponent*>(tuple.second);
        TeamComponent* otherTeam = std::get<TeamComponent*>(tuple.second);

        if(team->team != otherTeam->team)
          ai->UpdateFromOther(otherTransform, otherState);
      }
    }
  }
};
