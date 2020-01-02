#include "AssetManagement/Resource.h"

#include "Entity.h"
#include "GameManagement.h"

#include "Components/Sprite.h"
#include "Components/Camera.h"
#include "Components/Physics.h"
#include "Components/Collider.h"
#include "Components/GameActor.h"
#include "Components/ActionController.h"

#include <cassert>

//______________________________________________________________________________
void Sprite::Init(const char* sheet)
{
  //adds new operation to the blitting list
  ResourceManager::Get().RegisterBlitOp();
  _display = std::make_unique<Image>(sheet);
}

//______________________________________________________________________________
void Sprite::OnFrameBegin()
{
  _op = ResourceManager::Get().GetAvailableOp();
}

//______________________________________________________________________________
void Sprite::Update(float dt)
{
  _display->SetOp(_owner->transform, _display->GetRectOnSrcText(), Vector2<int>(0, 0), _op);
}

//______________________________________________________________________________
void Camera::Init(int w, int h)
{
  _rect.x = 0;
  _rect.y = 0;
  _rect.w = w;
  _rect.h = h;
}

//______________________________________________________________________________
void Camera::Update(float dt)
{
  _rect.x = static_cast<int>(std::floor(_owner->transform.position.x));
  _rect.y = static_cast<int>(std::floor(_owner->transform.position.y));

}

//______________________________________________________________________________
void Camera::ConvScreenSpace(ResourceManager::BlitOperation* entity)
{
  entity->_displayRect.x -= _rect.x;
  entity->_displayRect.y -= _rect.y;
}

//______________________________________________________________________________
bool Camera::EntityInDisplay(const ResourceManager::BlitOperation* entity)
{
  return SDLRectOverlap(_rect, entity->_displayRect);
}

//______________________________________________________________________________
void Physics::Update(float dt)
{
  _acc = Vector2<float>(0, Gravity);
  if (auto rCollider = _owner->GetComponent<RectColliderD>())
  {
    if (rCollider->IsStatic()) _acc = Vector2<float>(0, 0);
  }
  // Apply last frame of acceleration to the velocity
  _vel += _acc * dt;

  // Create the movement vector based on speed and acceleration of the object
  auto fix = [](float dt) { return (int)std::floor(10000 * dt) / 10000.0; };

  Vector2<double> movementVector = Vector2<double>(_vel.x * fix(dt), _vel.y * fix(dt));
  // Check collisions with other physics objects here and correct the movement vector based on those collisions
  Vector2<double> collisionAdjustmentVector = DoElasticCollisions(movementVector);

  Vector2<float> caVelocity = (Vector2<float>((float)(collisionAdjustmentVector.x) / (float)fix(dt), (float)(collisionAdjustmentVector.y) / (float)fix(dt)));
  // Convert adjustment vector to a velocity and change object's velocity based on the adjustment
  _vel += caVelocity;
  // Add the movement vector to the entityd
  _owner->transform.position += _vel * dt;

}

//______________________________________________________________________________
Vector2<double> Physics::DoElasticCollisions(const Vector2<double>& movementVector)
{
  auto CreateResolveCollisionVector = [this, &movementVector](OverlapInfo<double> overlap)
  {
    Vector2<double> resolutionVector(0.0, 0.0);
    //! Do each individually
    if ((overlap.collisionSides & CollisionSide::LEFT) != CollisionSide::NONE ||
      (overlap.collisionSides & CollisionSide::RIGHT) != CollisionSide::NONE)
    {
      resolutionVector.x = -overlap.amount.x;
    }
    if ((overlap.collisionSides & CollisionSide::UP) != CollisionSide::NONE ||
      (overlap.collisionSides & CollisionSide::DOWN) != CollisionSide::NONE)
    {
      resolutionVector.y = -overlap.amount.y;
    }

    // in the case that we're hitting a corner, only correct by the value that has the greater amount of overlap
    if (overlap.numCollisionSides > 1)
    {
      if (std::abs(overlap.amount.x) > std::abs(overlap.amount.y))
      {
        resolutionVector.x = 0;
      }
      else
      {
        resolutionVector.y = 0;
      }

      // if movement isnt in the same "direction" as the movement vector, dont adjust in that direction
      if ((overlap.amount.x > 0 && movementVector.x <= 0) || (overlap.amount.x < 0 && movementVector.x >= 0))
      {
        resolutionVector.x = 0;
      }
      if ((overlap.amount.y > 0 && movementVector.y < 0) || (overlap.amount.y < 0 && movementVector.y > 0))
      {
        resolutionVector.y = 0;
      }
    }

    return resolutionVector;
  };

  OverlapInfo<double> correction;

  auto myCollider = _owner->GetComponent<RectColliderD>();
  if (myCollider && !myCollider->IsStatic())
  {
    Rect<double> potentialRect = myCollider->rect;

    potentialRect = Rect<double>(Vector2<double>(_owner->transform.position.x, _owner->transform.position.y),
      Vector2<double>(_owner->transform.position.x + potentialRect.Width(),
        _owner->transform.position.y + potentialRect.Height()));

    potentialRect.Move(movementVector + correction.amount);

    for (auto collider : ComponentManager<RectColliderD>::Get().All())
    {
      if (myCollider != collider)
      {
        if (potentialRect.Collides(collider->rect))
        {
          //! return the reverse of the overlap to correct for the collision
          auto overlap = potentialRect.Overlap(collider->rect);
          
          //!
          //correction.numCollisionSides += overlap.numCollisionSides;
          correction.collisionSides |= overlap.collisionSides;
          correction.amount += CreateResolveCollisionVector(overlap);
        }
      }
    }
  }

  _lastCollisionSide = correction.collisionSides;
  return correction.amount;
}

//______________________________________________________________________________
template <typename T>
void RectCollider<T>::Init(Vector2<T> beg, Vector2<T> end)
{
  rect = Rect<T>(beg, end);
}

//______________________________________________________________________________
template <typename T>
void RectCollider<T>::Update(float dt)
{
  if (!_isStatic)
  {
    rect = Rect<T>(Vector2<T>((T)_owner->transform.position.x, (T)_owner->transform.position.y),
      Vector2<T>(_owner->transform.position.x + rect.Width(), _owner->transform.position.y + rect.Height()));
  }
}

//______________________________________________________________________________
void GameActor::Update(float dt)
{
  // action controller should take over state control
  if (auto action = _owner->GetComponent<ActionController>())
  {
    if (_owner->GetComponent<ActionController>()->IsPerformingAction())
      return;
  }


  if (auto physics = _owner->GetComponent<Physics>())
  {
    auto collisions = physics->GetLastCollisionSides();
    if (!HasState(collisions, CollisionSide::DOWN) && physics->_vel.y != 0)
    {
      if (physics->_vel.y > 0)
        _stance = StanceState::FALLING;
      else
        _stance = StanceState::JUMPING;
    }

    if (physics->_vel.x > 0)
      _moving = MovingState::FORWARD;
    else if (physics->_vel.x < 0)
      _moving = MovingState::BACKWARD;
    else
      _moving = MovingState::IDLE;
  }


  if (auto animator = _owner->GetComponent<Animator>())
  {
    if (_stance != StanceState::STANDING)
    {
      if(_stance == StanceState::CROUCHING)
        animator->Play("Crouch", true);
      else if (_stance == StanceState::JUMPING)
        animator->Play("Jumping", false);
      else
        animator->Play("Falling", false);
    }
    else
    {
      if (_moving == MovingState::FORWARD)
        animator->Play("WalkF", true);
      else if (_moving == MovingState::BACKWARD)
        animator->Play("WalkB", true);
      else
      {
        animator->Play("Idle", true);
      }
    }
  }

}

//______________________________________________________________________________
void GameActor::HandleMovementCommand(Vector2<float> movement)
{
  // make sure i can even act this frame
  // what are those things? am i in Hit Stun, Block Stun, Attack Animation (non cancellable), Attack Animation (cancellable, but not at the right time to cancel) 

  if (auto actionController = _owner->GetComponent<ActionController>())
  {
    InputState checkState = movement.y < 0 ? InputState::UP : InputState::NONE;
    bool handled = actionController->HandleInput(checkState);

    if (handled)
      return;
  }

  if(_controllableState)
  {
    auto vel = _baseSpeed * movement;
    assert(_owner->GetComponent<Physics>());
    _owner->GetComponent<Physics>()->_vel.x = vel.x;

    if (movement.y < 0 && HasState(_owner->GetComponent<Physics>()->GetLastCollisionSides(), CollisionSide::DOWN))
    {
      _owner->GetComponent<Physics>()->_vel.y = (1.5f) * vel.y;
    }
    else if (movement.y > 0 && HasState(_owner->GetComponent<Physics>()->GetLastCollisionSides(), CollisionSide::DOWN) && _stance != StanceState::CROUCHING)
    {
      _owner->GetComponent<ActionController>()->StartStateTransitionAction("Crouching", StanceState::CROUCHING);
    }
    else if (movement.y <= 0)
    {
      _stance = StanceState::STANDING;
    }
  }
}

//______________________________________________________________________________
void GameActor::HandleLightButtonCommand()
{
  if (auto actionController = _owner->GetComponent<ActionController>())
  {
    bool handled = actionController->HandleInput(InputState::BTN1);

    if (handled)
      return;
  }

  if (_controllableState && _owner->GetComponent<ActionController>())
  {
    if (_stance == StanceState::CROUCHING)
      _owner->GetComponent<ActionController>()->StartAnimatedAction("CL");
    else
      _owner->GetComponent<ActionController>()->StartAnimatedAction("SL");
  }
}

//______________________________________________________________________________
void GameActor::HandleStrongButtonCommand()
{
  if (auto actionController = _owner->GetComponent<ActionController>())
  {
    bool handled = actionController->HandleInput(InputState::BTN2);

    if (handled)
      return;
  }

  if (_controllableState && _owner->GetComponent<ActionController>())
  {
    if(_stance == StanceState::CROUCHING)
      _owner->GetComponent<ActionController>()->StartAnimatedAction("CS");
    else
      _owner->GetComponent<ActionController>()->StartAnimatedAction("SS");
  }
}


//______________________________________________________________________________
void GameActor::HandleHeavyButtonCommand()
{
  if (auto actionController = _owner->GetComponent<ActionController>())
  {
    bool handled = actionController->HandleInput(InputState::BTN3);

    if (handled)
      return;
  }

  if (_controllableState && _owner->GetComponent<ActionController>())
  {
    if (_stance == StanceState::CROUCHING)
      _owner->GetComponent<ActionController>()->StartAnimatedAction("CH", true);
    else
      _owner->GetComponent<ActionController>()->StartAnimatedAction("SH");
  }
}

//______________________________________________________________________________
void ActionController::Update(float dt)
{
  for (auto& action : _currentActions)
  {
    action->OnUpdate(_owner.get());
  }
}

//______________________________________________________________________________
void ActionController::OnFrameEnd()
{
  for (auto finished : _actionsFinished)
  {
    auto action = _currentActions.find(finished);
    if (action != _currentActions.end())
    {
      delete *action;
      _currentActions.erase(action);
    }
  }
  _actionsFinished.clear();
}

//______________________________________________________________________________
void ActionController::OnActionComplete(IAction* action)
{
  _actionsFinished.insert(action);
}

//______________________________________________________________________________
bool ActionController::HandleInput(InputState bttn)
{
  // cant handle the input if there isn't an action
  if (_currentActions.empty()) return false;

  return (*_currentActions.begin())->HandleInput(bttn, _owner.get());
}

//______________________________________________________________________________
void ActionController::StartAnimatedAction(const std::string& animName, bool isJC)
{
  AnimatedAction* action;
  if (!isJC)
    action = new AnimatedAction(animName, _owner.get());
  else
    action = new JumpCancellable(animName, _owner.get());

  _currentActions.insert(action);
}


template class RectCollider<double>;
