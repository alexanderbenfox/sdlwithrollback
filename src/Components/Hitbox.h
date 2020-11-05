#pragma once
#include "Core/ECS/IComponent.h"
#include "Components/Collider.h"
#include "Core/FightingGameTypes/HitData.h"
#include "AssetManagement/EditableAssets/FrameData.h"

class TransferDataBox : public RectColliderD
{
public:
  TransferDataBox() : hitFlag(false), RectColliderD() {}
  //! Data to transfer on hit
  HitData tData;
  //! Flag for if the data has been transfered already or not
  bool hitFlag;
  //! Initialize the transfer data using frame data
  virtual void Init(const FrameData& frameData);

  virtual void MoveDataBoxAroundTransform(const Transform* transform, const Rect<double>& box, const Vector2<float> dataOffsetFromTransformCenter, bool onLeft);

  virtual void Serialize(std::ostream& os) const override
  {
    RectColliderD::Serialize(os);
    Serializer<HitData>::Serialize(os, tData);
    Serializer<bool>::Serialize(os, hitFlag);
  }

  virtual void Deserialize(std::istream& is) override
  {
    RectColliderD::Deserialize(is);
    Serializer<HitData>::Deserialize(is, tData);
    Serializer<bool>::Deserialize(is, hitFlag);
  }
};

//! hitbox is the area that will hit the opponent
class Hitbox : public TransferDataBox
{
public:
  //!
  Hitbox() : TransferDataBox() {}

  bool travelWithTransform = false;

  bool destroyOnHit = false;

  virtual void OnCollision(const EntityID& entity, ICollider* collider) override;

  virtual void Serialize(std::ostream& os) const override
  {
    TransferDataBox::Serialize(os);
    Serializer<bool>::Serialize(os, travelWithTransform);
    Serializer<bool>::Serialize(os, destroyOnHit);
  }

  virtual void Deserialize(std::istream& is) override
  {
    TransferDataBox::Deserialize(is);
    Serializer<bool>::Deserialize(is, travelWithTransform);
    Serializer<bool>::Deserialize(is, destroyOnHit);
  }

};

template <> struct ComponentInitParams<Hitbox>
{
  Vector2<double> size;
  HitData hData;
  bool travelWithTransform;
  bool destroyOnHit;
  static void Init(Hitbox& component, const ComponentInitParams<Hitbox>& params)
  {
    component.rect = Rect<double>(0, 0, params.size.x, params.size.y);
    component.tData = params.hData;
    component.travelWithTransform = params.travelWithTransform;
    component.destroyOnHit = params.destroyOnHit;
  }
};

//! box that will initiate the throw action
class Throwbox : public TransferDataBox
{
public:
  //!
  Throwbox() : TransferDataBox() {}

  //! Resets thrown properties on state component
  void OnRemove(const EntityID& entity) override;

};

//! box that will track the movement of the throw while it happens
class ThrowFollower : public TransferDataBox
{
public:
  //!
  ThrowFollower() : TransferDataBox() {}
  //!
  bool startSideLeft = false;

  virtual void Serialize(std::ostream& os) const override
  {
    TransferDataBox::Serialize(os);
    Serializer<bool>::Serialize(os, startSideLeft);
  }

  virtual void Deserialize(std::istream& is) override
  {
    TransferDataBox::Deserialize(is);
    Serializer<bool>::Deserialize(is, startSideLeft);
  }

};
