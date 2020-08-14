#include "Components/Hitbox.h"
#include "Components/Hurtbox.h"
#include "Components/StateComponent.h"
#include "GameManagement.h"

void TransferDataBox::Init(const FrameData& frameData)
{
  // can act on the frame following the last recovery frame 
  int framesTilNeutral = frameData.active + frameData.recover + 1;

  //! frames in stun are defined by the frame that the player can ACT on following recieving this type of action

  // this is kind of a hack right now until i figure out why this doesnt work
  // problem is that you can't "act" on this frame for some reason so has to be on the next frame
  // PLEASE FIX LATER probably something in the way that AnimationListenerSystem is updated
  tData.framesInStunHit = framesTilNeutral + frameData.onHitAdvantage + 1;

  // does not happen for blocking because it is done through the TimedActionSystem
  tData.framesInStunBlock = framesTilNeutral + frameData.onBlockAdvantage;

  // transfer the rest of the attack data through this
  tData.damage = frameData.damage;
  tData.knockback = frameData.knockback;
  tData.activeFrames = frameData.active;
  tData.knockdown = frameData.knockdown;
}

void TransferDataBox::MoveDataBoxAroundTransform(const Rect<double>& unscaledTransformRect, const Transform* transform, const Rect<double>& box, const Vector2<float> offset, bool onLeft)
{
  Vector2<float> transCenterRelativeToAnim(unscaledTransformRect.HalfWidth() + offset.x, unscaledTransformRect.HalfHeight() + offset.y);
  Vector2<double> relativeToTransformCenter = box.GetCenter() - (Vector2<double>)transCenterRelativeToAnim;
  if (!onLeft)
    relativeToTransformCenter.x *= -1.0;

  rect = Rect<double>(0, 0, transform->scale.x * box.Width(), transform->scale.y * box.Height());
  rect.CenterOnPoint((Vector2<double>)transform->position + transform->scale * relativeToTransformCenter);
}

void Hitbox::OnCollision(const EntityID& entity, ICollider* collider)
{
  if (destroyOnHit)
  {
    GameManager::Get().TriggerEndOfFrame([entity]() {GameManager::Get().DestroyEntity(entity); });
  }
}

void Throwbox::OnRemove(const EntityID& entity)
{
  GameManager::Get().GetEntityByID(entity)->GetComponent<StateComponent>()->triedToThrowThisFrame = false;
  GameManager::Get().GetEntityByID(entity)->GetComponent<StateComponent>()->throwSuccess = false;
}
