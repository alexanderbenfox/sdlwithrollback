#pragma once
#include "Core/Geometry2D/Rect.h"

enum class CollisionSide : unsigned char
{
  NONE = 0x00,
  UP = 0x01,
  DOWN = 0x02,
  RIGHT = 0x04,
  LEFT = 0x08
};

//______________________________________________________________________________
void operator|=(CollisionSide& a, CollisionSide b);
//______________________________________________________________________________
CollisionSide operator|(CollisionSide a, CollisionSide b);
//______________________________________________________________________________
void operator&=(CollisionSide& a, CollisionSide b);
//______________________________________________________________________________
CollisionSide operator&(CollisionSide a, CollisionSide b);
//______________________________________________________________________________
CollisionSide operator~(CollisionSide a);
//______________________________________________________________________________
static bool HasState(const CollisionSide& state, CollisionSide other) { return (state & other) == other; }
//! Returns true if states share at least 1 common
static bool HasUnion(const CollisionSide& state, CollisionSide other) { return (state & other) != CollisionSide::NONE; }
//______________________________________________________________________________
std::ostream& operator<<(std::ostream& os, const CollisionSide& rb);
//______________________________________________________________________________
std::istream& operator>>(std::istream& is, CollisionSide& rb);


//______________________________________________________________________________
template <typename T>
struct OverlapInfo
{
  Vector2<T> amount;
  CollisionSide collisionSides;
  int numCollisionSides;

  //!
  OverlapInfo() : amount(0.0, 0.0), collisionSides(CollisionSide::NONE), numCollisionSides(0) {}
};

//______________________________________________________________________________
class RectHelper
{
public:
  template <typename T>
  static OverlapInfo<T> Overlap(const Rect<T>& a, const Rect<T>& b)
  {
    Rect<T> intersection = a.GetIntersection(b);
    // if the intersection left == this rectangle's left, then this is entering that rect from the left side
    // if the intersection bottom == this rectangle's bottom, then entering from above
    // return the vector of perterbation through the other rect
    OverlapInfo<T> info;
    info.collisionSides = CollisionSide::NONE;
    info.numCollisionSides = 0;

    if (!(intersection.beg.x == a.beg.x && intersection.end.x == a.end.x))
    {
      if (intersection.beg.x == a.beg.x)
      {
        info.collisionSides |= CollisionSide::LEFT;
        info.numCollisionSides++;
      }
      else if (intersection.end.x == a.end.x)
      {
        info.collisionSides |= CollisionSide::RIGHT;
        info.numCollisionSides++;
      }
    }
    if (!(intersection.beg.y == a.beg.y && intersection.end.y == a.end.y))
    {
      if (intersection.end.y == a.end.y)
      {
        info.collisionSides |= CollisionSide::DOWN;
        info.numCollisionSides++;
      }
      else if (intersection.beg.y == a.beg.y)
      {
        info.collisionSides |= CollisionSide::UP;
        info.numCollisionSides++;
      }
    }

    info.amount = Vector2<T>(intersection.beg.x == a.beg.x ? -intersection.Width() : intersection.Width(),
      intersection.end.y == a.end.y ? intersection.Height() : -intersection.Height());

    return info;
  }
};
