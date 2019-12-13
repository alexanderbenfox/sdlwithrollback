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


  // Create the movement vector based on speed and acceleration of the object
  auto fix = [](float dt) { return (int)std::floorf(10000 * dt) / 10000.0; };

  Vector2<double> movementVector = Vector2<double>(_vel.x * fix(dt), _vel.y * fix(dt));
  // Check collisions with other physics objects here and correct the movement vector based on those collisions
  Vector2<double> collisionAdjustmentVector = DoElasticCollisions(movementVector);

  Vector2<float> caVelocity = (Vector2<float>((float)(collisionAdjustmentVector.x)/ fix(dt), (float)(collisionAdjustmentVector.y)/ fix(dt)));
  // Convert adjustment vector to a velocity and change object's velocity based on the adjustment
  _vel += caVelocity;
  // Add the movement vector to the entity
  _owner->transform.position += _vel * dt;
  // Apply last frame of acceleration to the velocity
  _vel += _acc * dt;
}

//______________________________________________________________________________
Vector2<double> Physics::DoElasticCollisions(const Vector2<double>& movementVector)
{
  _lastCollisionSide = CollisionSide::NONE;

  Vector2<double> fixVector(0, 0);

  auto myCollider = _owner->GetComponent<RectColliderD>();
  if (myCollider && !myCollider->IsStatic())
  {
    bool checkCollision = true;

    while (checkCollision)
    {
      checkCollision = false;

      Rect<double> potentialRect = myCollider->rect;

      potentialRect = Rect<double>(Vector2<double>(_owner->transform.position.x, _owner->transform.position.y),
        Vector2<double>(_owner->transform.position.x + potentialRect.Width(),
          _owner->transform.position.y + potentialRect.Height()));

      potentialRect.Move(movementVector + fixVector);

      for (auto collider : ComponentManager<RectColliderD>::Get().All())
      {
        if (myCollider != collider)
        {
          if (potentialRect.Collides(collider->rect))
          {
            //! return the reverse of the overlap to correct for the collision
            fixVector += (-1.0 * potentialRect.Overlap(collider->rect, movementVector));
            //!
            _lastCollisionSide = (CollisionSide)( (unsigned char)_lastCollisionSide | (unsigned char)(fixVector.x > 0 ? CollisionSide::LEFT : (fixVector.x < 0 ? CollisionSide::RIGHT : (fixVector.y < 0 ? CollisionSide::DOWN : (fixVector.y > 0 ? CollisionSide::UP : CollisionSide::NONE)))));
            // since the collision will move the object, we need to check those collisions as well
            checkCollision = true;
          }
        }
      }
    }
  }
  return fixVector;
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
void GameActor::HandleMovementCommand(Vector2<float> movement)
{
  if(_controllableState)
  {
    auto vel = _baseSpeed * movement;
    assert(_owner->GetComponent<Physics>());
    _owner->GetComponent<Physics>()->_vel.x = vel.x;

    if (movement.y < 0 && ((unsigned char)_owner->GetComponent<Physics>()->GetLastCollisionSides() & (unsigned char)CollisionSide::DOWN) != 0)
    {
      _owner->GetComponent<Physics>()->_vel.y = (1.2f) * vel.y;
    }
  }
}

template class RectCollider<double>;
