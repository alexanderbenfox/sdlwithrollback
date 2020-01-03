#include "Actions.h"

//all of the possible states for animation??
enum class StanceState
{
  CROUCHING, STANDING, JUMPING
};

enum class ActionState
{
  NEUTRAL, BLOCKSTUN, HITSTUN, DASHING, SHORT, STRONG, FIERCE
};

struct State
{
  StanceState stance;
  ActionState action;
};

struct HitInfo
{
  int onHit, onBlock;
  bool isOverhead;
  bool isLow;

  IStateNode* ResolveGroundedHitState(InputState input, bool wasHit, bool attackingFromRight)
  {
    //grounded hit actions

    if(wasHit)
    {
      bool hitBlocked = (attackingFromRight && HasState(input, InputState::LEFT)) ||
         (!attackingFromRight && HasState(input, InputState::RIGHT));

      if(HasState(input, InputState::DOWN))
      {
        // if got hit overhead and player is still crouching, it is a hit - if got hit and pressing down
        if(isOverhead || !hitBlocked)
          return new CrouchingHitstun;
        return new CrouchingBlockstun;
      }
      else
      {
        // if got hit low and the player isn't crouching, it is a hit
        if(isLow || !hitBlocked)
          return new StandingHitstun;
        return new StandingBlockstun;
      }
    }
    return nullptr;
  }
};

struct GameInput
{
  CollisionState collision;
  HitInfo hit;
  InputState bttns;
};

class IStateNode
{
public:
  IStateNode* Transition(GameInput lastFrame) = 0;
  // assuming that you weren't hit and that you're free to make a new action
  IStateNode* ResolveGroundedNeutralActions(InputState state)
  {
    // prioritize attacks
    if(HasState(input, InputState::BTTN1))
    {
      if(HasState(input, InputState::DOWN))
        return new CrouchingShort;
      else
        return new StandingShort;
    }
      
    else if (HasState(input, InputState::BTTN2))
    {
      if(HasState(input, InputState::DOWN))
        return new CrouchingStrong;
      else
        return new StandingStrong;
    }
      
    else if (HasState(input, InputState::BTTN3))
    {
      if(HasState(input, InputState::DOWN))
        return new CrouchingFierce;
      else
        return new StandingFierce;
    }

    //if you arent attacking, you can move forward, move backward, crouch, stand, jumpf, jumpb, jumpn
    //jumping
    if(HasState(input, InputState::UP))
    {
      if(HasState(input, InputState::LEFT))
        return new JumpingBack;
      else if (HasState(input, InputState::RIGHT))
        return new JumpingForward;
      return new JumpingNeutral;
    }
    if(HasState(input, InputState::DOWN))
    {
      return new CrouchingNeutral;
    }
    if(HasState(input, InputState::LEFT))
      return new WalkLeft;
    else if (HasState(input, InputState::RIGHT))
      return new WalkRight;
    return new StandingNeutral;
  }
};

class CrouchingNeutral : public IStateNode
{
public:
  IStateNode* Transition(GameInput lastFrame) override
  {
    IStateNode* groundedHit = lastFrame.hit.ResolveGroundedHitState(lastFrame.bttns);
    if(groundedHit)
      return groundedHit;
    //since collision state shouldnt matter here, just resolve neutral actions
    return ResolveGroundedNeutralActions(lastFrame.bttns);
  }
private:
  State state = {StanceState::CROUCHING, ActionState::NEUTRAL};
}

class StandingNeutral : public IStateNode
{
public:
  IStateNode* Transition(GameInput lastFrame) override
  {
    IStateNode* groundedHit = lastFrame.hit.ResolveGroundedHitState(lastFrame.bttns);
    if(groundedHit)
      return groundedHit;
    //since collision state shouldnt matter here, just resolve neutral actions
    return ResolveGroundedNeutralActions(lastFrame.bttns);
  }
private:
  State state = {StanceState::STANDING, ActionState::NEUTRAL};
}