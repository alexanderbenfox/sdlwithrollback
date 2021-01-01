#include "Core/ECS/ISystem.h"
#include "Components/AIComponent.h"

class UpdateAISystem : public IMultiSystem<SysComponents<AIComponent, Transform, StateComponent, TeamComponent>, SysComponents<Transform, StateComponent, TeamComponent>>
{
public:
  static void DoTick(float dt)
  {
    PROFILE_FUNCTION();
    for (const EntityID& e1 : MainSystem::Registered)
    {
      AIComponent& ai = ComponentArray<AIComponent>::Get().GetComponent(e1);
      Transform& transform = ComponentArray<Transform>::Get().GetComponent(e1);
      StateComponent& state = ComponentArray<StateComponent>::Get().GetComponent(e1);
      TeamComponent& team = ComponentArray<TeamComponent>::Get().GetComponent(e1);

      ai.UpdateMyState(&transform, &state);
      
      for(const EntityID& e2 : SubSystem::Registered)
      {
        Transform& otherTransform = ComponentArray<Transform>::Get().GetComponent(e2);
        StateComponent& otherState = ComponentArray<StateComponent>::Get().GetComponent(e2);
        TeamComponent& otherTeam = ComponentArray<TeamComponent>::Get().GetComponent(e2);

        if(team.team != otherTeam.team)
          ai.UpdateFromOther(&otherTransform, &otherState);
      }
    }
  }
};
