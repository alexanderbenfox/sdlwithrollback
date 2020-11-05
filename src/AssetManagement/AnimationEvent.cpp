#include "AssetManagement/AnimationEvent.h"
#include "Components/Hitbox.h"
#include "Components/Hurtbox.h"
#include "Components/Rigidbody.h"
#include "Managers/GameManagement.h"

#include "Systems/DestroyEntitiesSystem.h"

//______________________________________________________________________________
void AnimationEvent::EndHitboxEvent(EntityID entity)
{
  GameManager::Get().GetEntityByID(entity)->RemoveComponent<Hitbox>();
}

//______________________________________________________________________________
void AnimationEvent::EndThrowboxEvent(EntityID entity)
{
  GameManager::Get().GetEntityByID(entity)->RemoveComponent<Throwbox>();
  GameManager::Get().GetEntityByID(entity)->RemoveComponent<ThrowFollower>();
}

//______________________________________________________________________________
void AnimationEvent::EndMovementEvent(EntityID entity)
{

}

//______________________________________________________________________________
void AnimationEvent::EndEntitySpawnEvent(EntityID entity)
{

}

//______________________________________________________________________________
EventList AnimationEventHelper::BuildEventList(const Vector2<float>& textureScalingFactor, const Vector2<float> texToCornerOffset, const std::vector<EventData>& animEventData, const FrameData& frameData, int totalSheetFrames, std::vector<int>& animFrameToSheetFrame, AnchorPoint animAnchorPt)
{
  Vector2<float> offset(texToCornerOffset.x + m_characterWidth / 2.0f, texToCornerOffset.y + m_characterHeight / 2.0f);

  if (animAnchorPt == AnchorPoint::TL)
  {

  }
  else if (animAnchorPt == AnchorPoint::BL)
  {
    offset.x = texToCornerOffset.x + m_characterWidth / 2.0f;
    offset.y = texToCornerOffset.y - m_characterHeight / 2.0f;
  }
  else if (animAnchorPt == AnchorPoint::TR)
  {
    offset.x = texToCornerOffset.x - m_characterWidth / 2.0f;
    offset.y = texToCornerOffset.y + m_characterHeight / 2.0f;
  }
  else
  {
    offset.x = texToCornerOffset.x - m_characterWidth / 2.0f;
    offset.y = texToCornerOffset.y - m_characterHeight / 2.0f;
  }

  auto DespawnHitbox = [](EntityID entity) { GameManager::Get().GetEntityByID(entity)->RemoveComponent<Hitbox>(); };
  auto DespawnThrowStuff = [](EntityID entity)
  {
    GameManager::Get().GetEntityByID(entity)->RemoveComponent<Throwbox>();
    GameManager::Get().GetEntityByID(entity)->RemoveComponent<ThrowFollower>();
  };
  auto EndMovement = [](EntityID) {};

  std::function<void(EntityID, Transform*, StateComponent*)> trigger;
  std::vector<std::function<void(EntityID, Transform*, StateComponent*)>> updates;

  EventBuilderDictionary animationData = ParseAnimationEventList(animEventData, frameData, totalSheetFrames);

  if (animationData.sheetFrameToRealFrame.size() != animEventData.size())
  {
    // log some kind of error here and return
  }

  animFrameToSheetFrame = animationData.realFrameToSheetFrame;
  int animFrames = animEventData.size();
  int realFrames = animationData.realFrameToSheetFrame.size();

  EventList eventList;
  eventList.resize(realFrames);

  int startFrame = 0;
  int counter = 0;

  auto addEventToList = [&startFrame, &counter, &updates, &eventList, &trigger](const std::function<void(EntityID)>& onComplete, AnimationEvent::Type type)
  {
    eventList[startFrame].emplace_back(startFrame, counter, trigger, updates, onComplete, type);
    updates.clear();
    counter = 0;
    startFrame = 0;
  };

  auto eventCheck = [addEventToList, &startFrame, &counter, &eventList, &animationData, &trigger, &updates]
  (int i, const std::function<void(EntityID)>& onComplete, const std::function<void(EntityID, Transform*, StateComponent*)>& callback, bool conditionMet, AnimationEvent::Type type,
    std::function<void(EntityID, Transform*, StateComponent*)>* onTrigger = nullptr)
  {
    if (conditionMet)
    {
      if (counter == 0)
      {
        int finder = 0;
        while (animationData.sheetFrameToRealFrame[i + finder].empty())
        {
          finder++;
          if (i + finder == animationData.sheetFrameToRealFrame.size())
          {
            finder--;
            break;
          }
        }
        startFrame = animationData.sheetFrameToRealFrame[i + finder][0];
        if (onTrigger)
        {
          trigger = *onTrigger;
          counter++;
          return;
        }
        else
          trigger = callback;
      }


      bool isTrigger = counter == 0;
      for (const int& realFrame : animationData.sheetFrameToRealFrame[i])
      {
        counter++;
        //avoid adding one to update if this is a trigger frame
        if (isTrigger && onTrigger)
        {
          if (counter == 1)
            continue;
          else
            updates.push_back(*onTrigger);
        }
        else
        {
          if (isTrigger)
          {
            isTrigger = false;
            continue;
          }
          updates.push_back(callback);
        }
      }
    }
    else if (counter > 0)
    {
      addEventToList(onComplete, type);
    }
  };


  for (int i = 0; i < animFrames; i++)
  {
    Rect<double> hitbox = animEventData[i].hitbox;
    hitbox.beg *= textureScalingFactor;
    hitbox.end *= textureScalingFactor;

    bool hitboxCondition = hitbox.Area() != 0;

    if (frameData.isThrow)
    {
      // add normal hitbox data
      std::function<void(EntityID, Transform*, StateComponent*)> throwInitiate = [hitbox, frameData, offset](EntityID entity, Transform* trans, StateComponent* state)
      {
        GameManager::Get().GetEntityByID(entity)->AddComponent<Throwbox>();
        GameManager::Get().GetEntityByID(entity)->GetComponent<Throwbox>()->Init(frameData);
        GameManager::Get().GetEntityByID(entity)->GetComponent<Throwbox>()->MoveDataBoxAroundTransform(trans, hitbox, offset, state->onLeftSide);

        state->triedToThrowThisFrame = true;
      };

      std::function<void(EntityID, Transform*, StateComponent*)> throwUpdate = [hitbox, frameData, offset](EntityID entity, Transform* trans, StateComponent* state)
      {
        bool throwSuccess = true;
        if (GameManager::Get().GetEntityByID(entity)->GetComponent<Throwbox>())
        {
          throwSuccess = GameManager::Get().GetEntityByID(entity)->GetComponent<Throwbox>()->hitFlag;
          GameManager::Get().GetEntityByID(entity)->RemoveComponent<Throwbox>();
          GameManager::Get().GetEntityByID(entity)->AddComponent<ThrowFollower>();
          GameManager::Get().GetEntityByID(entity)->GetComponent<ThrowFollower>()->startSideLeft = state->onLeftSide;
        }

        if (throwSuccess)
        {
          GameManager::Get().GetEntityByID(entity)->GetComponent<ThrowFollower>()->Init(frameData);
          GameManager::Get().GetEntityByID(entity)->GetComponent<ThrowFollower>()->MoveDataBoxAroundTransform(trans, hitbox, offset, GameManager::Get().GetEntityByID(entity)->GetComponent<ThrowFollower>()->startSideLeft);
        }
      };
      eventCheck(i, DespawnThrowStuff, throwUpdate, hitboxCondition, AnimationEvent::Type::Throwbox, &throwInitiate);
    }
    else
    {
      // add normal hitbox data
      std::function<void(EntityID, Transform*, StateComponent*)> hitboxUpdateFunc = [hitbox, frameData, offset](EntityID entity, Transform* trans, StateComponent* state)
      {
        GameManager::Get().GetEntityByID(entity)->AddComponent<Hitbox>();
        GameManager::Get().GetEntityByID(entity)->GetComponent<Hitbox>()->Init(frameData);
        GameManager::Get().GetEntityByID(entity)->GetComponent<Hitbox>()->MoveDataBoxAroundTransform(trans, hitbox, offset, state->onLeftSide);
      };

      eventCheck(i, DespawnHitbox, hitboxUpdateFunc, hitboxCondition, AnimationEvent::Type::Hitbox);
    }
  }

  if (counter > 0)
    addEventToList(DespawnHitbox, AnimationEvent::Type::Hitbox);

  for (int i = 0; i < animFrames; i++)
  {
    const Vector2<float>& movement = animEventData[i].movement;
    bool mvmtCondition = movement.x != 0 || movement.y != 0;
    auto movementEvent = [movement](EntityID entity, Transform* trans, StateComponent* state)
    {
      if (auto rb = GameManager::Get().GetEntityByID(entity)->GetComponent<Rigidbody>())
      {
        auto move = movement;
        if (!state->onLeftSide)
          move.x *= -1.0f;
        rb->velocity += move;
      }
    };

    eventCheck(i, EndMovement, movementEvent, mvmtCondition, AnimationEvent::Type::Movement);
  }

  if (counter > 0)
    addEventToList(EndMovement, AnimationEvent::Type::Movement);

  // create entity section
  for (int i = 0; i < animFrames; i++)
  {
    const EntityCreationData& data = animEventData[i].create;
    auto DestroyCreatedEntity = [](EntityID){};

    if (!data.instructions.empty())
    {
      auto creationEvent = [data](EntityID entity, Transform* trans, StateComponent* state)
      {
        std::shared_ptr<Entity> eventEntity = GameManager::Get().CreateEntity<DestroyOnSceneEnd>();
        data.AddComponents(entity, trans, state, eventEntity);
        GameManager::Get().AddToNetworkedList(eventEntity->GetID());
      };

      int finder = 0;
      while (animationData.sheetFrameToRealFrame[i + finder].empty())
        finder++;

      startFrame = animationData.sheetFrameToRealFrame[i + finder][0];
      eventList[startFrame].emplace_back(startFrame, 1, creationEvent, updates, DestroyCreatedEntity, AnimationEvent::Type::EntitySpawner);
    }
  }

  return eventList;
}

//______________________________________________________________________________
EventBuilderDictionary AnimationEventHelper::ParseAnimationEventList(const std::vector<EventData>& animEventData, const FrameData& frameData, int totalSheetFrames)
{
  // creating a new list of frames to play to adjust for modified start up, active, and recovery values
  EventBuilderDictionary data;
  data.sheetFrameToRealFrame.resize(totalSheetFrames);

  int startUpFrames = -1;
  int activeFrames = -1;
  int recoveryFrames = -1;

  // first find where the hitbox comes out so we know our first active frame
  for (int i = 0; i < animEventData.size(); i++)
  {
    if (animEventData[i].isActive)
    {
      if (startUpFrames != -1)
      {
        activeFrames = i - startUpFrames + 1;
      }
      else
      {
        startUpFrames = i;
        activeFrames = i - startUpFrames + 1;
      }
    }
  }
  if (activeFrames != -1)
    recoveryFrames = totalSheetFrames - (startUpFrames + activeFrames);


  // using data for frame counts, construct the animated event in place
  if (startUpFrames > 0)
  {
    int animLastStartUpFrameIdx = startUpFrames - 1;
    int animLastActiveFrameIdx = animLastStartUpFrameIdx + activeFrames;
    // allow for a frame of "active" to seep into recovery for hitstop effect
    int animLastRecoveryFrameIdx = animLastActiveFrameIdx + recoveryFrames - 1;

    int lastStartUpFrameIdx = frameData.startUp - 2;
    int lastActiveFrameIdx = lastStartUpFrameIdx + frameData.active;
    int lastRecoveryFrameIdx = lastActiveFrameIdx + frameData.recover;

    int totalFramesAdjusted = lastRecoveryFrameIdx + 1;
    data.realFrameToSheetFrame.resize(totalFramesAdjusted);

    for (int i = 0; i < totalFramesAdjusted; i++)
    {

      // set up the pre active frames
      if (i <= lastStartUpFrameIdx)
      {
        if (lastStartUpFrameIdx == 0)
          data.realFrameToSheetFrame[i] = 0;
        else
          data.realFrameToSheetFrame[i] = (int)std::ceil((static_cast<double>(i) / static_cast<double>(lastStartUpFrameIdx)) * static_cast<double>(animLastStartUpFrameIdx));
        
      }
      else if (i <= lastActiveFrameIdx)
      {
        double idx = static_cast<double>(i - lastStartUpFrameIdx);
        data.realFrameToSheetFrame[i] = (int)std::ceil((idx / (double)frameData.active) * static_cast<double>(activeFrames)) + animLastStartUpFrameIdx;
      }
      else
      {
        double idx = static_cast<double>(i - lastActiveFrameIdx);
        data.realFrameToSheetFrame[i] = (int)std::ceil((idx / (double)frameData.recover) * static_cast<double>(recoveryFrames)) + animLastActiveFrameIdx;
      }

      // after we've established which animation frame maps to which in game frame, record that data as well
      data.sheetFrameToRealFrame[data.realFrameToSheetFrame[i]].push_back(i);
    }
  }
  return data;
}