#include "AssetManagement/Resource.h"

#include "Entity.h"
#include "GameManagement.h"

#include "Components/Sprite.h"
#include "Components/Camera.h"
#include "Components/Physics.h"
#include "Components/Collider.h"
#include "Components/GameActor.h"

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
  _display->SetOp(_owner->transform, _display->GetRectOnSrcText(), _op);
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
  _rect.x = static_cast<int>(std::floorf(_owner->transform.position.x));
  _rect.y = static_cast<int>(std::floorf(_owner->transform.position.y));

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
  auto fix = [](float dt) { return (int)std::floorf(10000 * dt) / 10000.0; };

  Vector2<double> movementVector = Vector2<double>(_vel.x * fix(dt), _vel.y * fix(dt));
  // Check collisions with other physics objects here and correct the movement vector based on those collisions
  Vector2<double> collisionAdjustmentVector = DoElasticCollisions(movementVector);

  Vector2<float> caVelocity = (Vector2<float>((float)(collisionAdjustmentVector.x) / fix(dt), (float)(collisionAdjustmentVector.y) / fix(dt)));
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

void GameActor::Update(float dt)
{
  if (auto physics = _owner->GetComponent<Physics>())
  {
    auto collisions = physics->GetLastCollisionSides();
    if (HasState(collisions, CollisionSide::DOWN))
      _aerial = AerialState::GROUNDED;
    else if (physics->_vel.y != 0)
    {
      if (physics->_vel.y > 0)
        _aerial = AerialState::FALLING;
      else
        _aerial = AerialState::JUMPING;
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
    if (_aerial != AerialState::GROUNDED)
    {
      if (_aerial == AerialState::JUMPING)
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
        animator->Play("Idle", true);
    }
  }

}

//______________________________________________________________________________
void GameActor::HandleMovementCommand(Vector2<float> movement)
{
  if(_controllableState)
  {
    auto vel = _baseSpeed * movement;
    assert(_owner->GetComponent<Physics>());
    _owner->GetComponent<Physics>()->_vel.x = vel.x;

    if (movement.y < 0 && ((unsigned char)_owner->GetComponent<Physics>()->GetLastCollisionSides() & (unsigned char)CollisionSide::DOWN) != 0)
    {
      _owner->GetComponent<Physics>()->_vel.y = (1.5f) * vel.y;
    }
  }
}

template class RectCollider<double>;
