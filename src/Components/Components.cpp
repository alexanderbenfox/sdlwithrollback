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

  _textureResource = &ResourceManager::Get().GetTexture(sheet);

  _sourceRect.x = 0;
  _sourceRect.y = 0;
  SDL_QueryTexture(_textureResource->Get(), nullptr, nullptr, &_sourceRect.w, &_sourceRect.h);
}

//______________________________________________________________________________
void Sprite::OnFrameBegin()
{
  _op = ResourceManager::Get().GetAvailableOp();
}

//______________________________________________________________________________
void Sprite::Update(float dt)
{
  _op->_textureRect = _sourceRect;
  _op->_textureResource = _textureResource;

  Transform& transform = _owner->transform;
  _op->_displayRect = { static_cast<int>(std::floorf(transform.position.x)), static_cast<int>(std::floorf(transform.position.y)),
    (int)(static_cast<float>(_sourceRect.w) * transform.scale.x),
    (int)(static_cast<float>(_sourceRect.h) * transform.scale.y) };

  _op->valid = true;
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
  if (auto rCollider = _owner->GetComponent<RectCollider>())
  {
    if (rCollider->IsStatic()) _acc = Vector2<float>(0, 0);
  }
  // Create the movement vector based on speed and acceleration of the object
  Vector2<float> movementVector = _vel * dt;
  // Check collisions with other physics objects here and correct the movement vector based on those collisions
  Vector2<float> collisionAdjustmentVector = DoElasticCollisions(movementVector);
  // Convert adjustment vector to a velocity and change object's velocity based on the adjustment
  _vel += (Vector2<float>(collisionAdjustmentVector.x / dt, collisionAdjustmentVector.y / dt));
  // Add the movement vector to the entity
  _owner->transform.position += _vel * dt;
  // Apply last frame of acceleration to the velocity
  _vel += _acc * dt;
}

//______________________________________________________________________________
Vector2<float> Physics::DoElasticCollisions(const Vector2<float>& movementVector)
{
  Vector2<float> fixVector(0, 0);

  auto myCollider = _owner->GetComponent<RectCollider>();
  if (myCollider && !myCollider->IsStatic())
  {
    bool checkCollision = true;

    while (checkCollision)
    {
      checkCollision = false;

      Rect<float> potentialRect = myCollider->rect;

      potentialRect = Rect<float>(_owner->transform.position,
        Vector2<float>(_owner->transform.position.x + potentialRect.Width(),
          _owner->transform.position.y + potentialRect.Height()));

      potentialRect.Move(movementVector + fixVector);

      for (auto collider : ComponentManager<RectCollider>::Get().All())
      {
        if (myCollider != collider)
        {
          if (potentialRect.Collides(collider->rect))
          {
            //! return the reverse of the overlap to correct for the collision
            fixVector += (-1.01f * potentialRect.Overlap(collider->rect, movementVector));
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
void RectCollider::Init(Vector2<float> beg, Vector2<float> end)
{
  rect = Rect<float>(beg, end);
}

//______________________________________________________________________________
void RectCollider::Update(float dt)
{
  if (!_isStatic)
  {
    rect = Rect<float>(_owner->transform.position,
      Vector2<float>(_owner->transform.position.x + rect.Width(), _owner->transform.position.y + rect.Height()));
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
  }
}
