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
ActionTimeline AnimationEventHelper::ResolveSpriteTimeline(
    const std::vector<EventData>& spriteFrameEvents,
    const FrameData& frameData,
    int totalSheetFrames,
    const Vector2<float>& textureScalingFactor,
    AnchorPoint anchorPt,
    const Vector2<float>& scaledAnchorOffset)
{
  ActionTimeline timeline;
  timeline.frameData = frameData;
  timeline.hitboxOffset = -CalculateRenderOffset(anchorPt, scaledAnchorOffset, Vector2<float>(m_characterWidth, m_characterHeight));

  EventBuilderDictionary mapping = ParseAnimationEventList(spriteFrameEvents, frameData, totalSheetFrames);

  int totalGameFrames = static_cast<int>(mapping.realFrameToSheetFrame.size());
  timeline.frames.resize(totalGameFrames);
  timeline.visualFrameMap = std::move(mapping.realFrameToSheetFrame);

  int numSpriteEvents = static_cast<int>(spriteFrameEvents.size());

  // Track which sheet frames have already had their spawn data copied —
  // entity spawns are one-shot events, not per-frame like hitboxes/movement.
  std::vector<bool> spawnCopied(numSpriteEvents, false);

  for (int gameFrame = 0; gameFrame < totalGameFrames; gameFrame++)
  {
    int sheetFrame = timeline.visualFrameMap[gameFrame];
    if (sheetFrame >= numSpriteEvents)
      continue;

    const EventData& src = spriteFrameEvents[sheetFrame];
    GameFrameEvent& dst = timeline.frames[gameFrame];

    dst.hitbox = src.hitbox;
    dst.hitbox.beg *= textureScalingFactor;
    dst.hitbox.end *= textureScalingFactor;

    dst.movement = src.movement;
    dst.isActive = src.isActive;

    // Only copy entity spawn data to the first game frame per sheet frame
    if (!src.create.IsEmpty() && !spawnCopied[sheetFrame])
    {
      dst.create = src.create;
      spawnCopied[sheetFrame] = true;
    }
  }

  return timeline;
}

//______________________________________________________________________________
EventList AnimationEventHelper::BuildEventList(const ActionTimeline& timeline)
{
  const auto& frames = timeline.frames;
  const auto& frameData = timeline.frameData;
  const auto& offset = timeline.hitboxOffset;

  auto DespawnHitbox = [](EntityID entity) { GameManager::Get().GetEntityByID(entity)->RemoveComponent<Hitbox>(); };
  auto DespawnThrowStuff = [](EntityID entity)
  {
    GameManager::Get().GetEntityByID(entity)->RemoveComponent<Throwbox>();
    GameManager::Get().GetEntityByID(entity)->RemoveComponent<ThrowFollower>();
  };
  auto EndMovement = [](EntityID) {};

  std::function<void(EntityID, Transform*, StateComponent*)> trigger;
  std::vector<std::function<void(EntityID, Transform*, StateComponent*)>> updates;

  int totalFrames = static_cast<int>(frames.size());

  EventList eventList;
  eventList.resize(totalFrames);

  int startFrame = 0;
  int counter = 0;

  auto addEventToList = [&startFrame, &counter, &updates, &eventList, &trigger](const std::function<void(EntityID)>& onComplete, AnimationEvent::Type type)
  {
    eventList[startFrame].emplace_back(startFrame, counter, trigger, updates, onComplete, type);
    updates.clear();
    counter = 0;
    startFrame = 0;
  };

  auto eventCheck = [addEventToList, &startFrame, &counter, &trigger, &updates]
  (int i, const std::function<void(EntityID)>& onComplete, const std::function<void(EntityID, Transform*, StateComponent*)>& callback, bool conditionMet, AnimationEvent::Type type,
    std::function<void(EntityID, Transform*, StateComponent*)>* onTrigger = nullptr)
  {
    if (conditionMet)
    {
      if (counter == 0)
      {
        startFrame = i;
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
      counter++;
      if (isTrigger)
        ; // trigger frame already handled above, don't add to updates
      else
        updates.push_back(callback);
    }
    else if (counter > 0)
    {
      addEventToList(onComplete, type);
    }
  };

  // Hitbox / throwbox events
  for (int i = 0; i < totalFrames; i++)
  {
    const Rect<double>& hitbox = frames[i].hitbox;
    bool hitboxCondition = hitbox.Area() != 0;

    if (frameData.isThrow)
    {
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
    addEventToList(frameData.isThrow ? DespawnThrowStuff : DespawnHitbox,
                   frameData.isThrow ? AnimationEvent::Type::Throwbox : AnimationEvent::Type::Hitbox);

  // Movement events
  for (int i = 0; i < totalFrames; i++)
  {
    const Vector2<float>& movement = frames[i].movement;
    bool mvmtCondition = movement.x != 0 || movement.y != 0;
    auto movementEvent = [movement](EntityID entity, Transform* trans, StateComponent* state)
    {
      if (auto rb = GameManager::Get().GetEntityByID(entity)->GetComponent<Rigidbody>())
      {
        auto move = movement;
        if (!state->onLeftSide)
          move.x *= -1.0f;
        rb->velocity.x += move.x;
        rb->velocity.y = move.y;
      }
    };

    eventCheck(i, EndMovement, movementEvent, mvmtCondition, AnimationEvent::Type::Movement);
  }

  if (counter > 0)
    addEventToList(EndMovement, AnimationEvent::Type::Movement);

  // Entity spawn events
  for (int i = 0; i < totalFrames; i++)
  {
    const ProjectileData& data = frames[i].create;
    auto DestroyCreatedEntity = [](EntityID){};

    if (!data.IsEmpty())
    {
      auto creationEvent = [data](EntityID entity, Transform* trans, StateComponent* state)
      {
        std::shared_ptr<Entity> eventEntity = GameManager::Get().CreateEntity<DestroyOnSceneEnd>();
        data.AddComponents(entity, trans, state, eventEntity);
        GameManager::Get().AddToNetworkedList(eventEntity->GetID());
      };

      std::vector<std::function<void(EntityID, Transform*, StateComponent*)>> emptyUpdates;
      eventList[i].emplace_back(i, 1, creationEvent, emptyUpdates, DestroyCreatedEntity, AnimationEvent::Type::EntitySpawner);
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
        if (startUpFrames == 0)
          startUpFrames = 1;
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