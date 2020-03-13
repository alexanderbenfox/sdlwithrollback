#include "Geometry.h"
#include <cmath>


//______________________________________________________________________________
void operator|=(CollisionSide& the, CollisionSide other)
{
  the = (CollisionSide)((unsigned char)the | (unsigned char)other);
}

//______________________________________________________________________________
CollisionSide operator|(CollisionSide a, CollisionSide b)
{
  return (CollisionSide)((unsigned char)a | (unsigned char)b);
}

//______________________________________________________________________________
void operator&=(CollisionSide& the, CollisionSide other)
{
  the = (CollisionSide)((unsigned char)the & (unsigned char)other);
}

//______________________________________________________________________________
CollisionSide operator&(CollisionSide a, CollisionSide b)
{
  return (CollisionSide)((unsigned char)a & (unsigned char)b);
}

//______________________________________________________________________________
CollisionSide operator~(CollisionSide og)
{
  return (CollisionSide)~((unsigned char)og);
}


//______________________________________________________________________________
template <typename T>
Vector2<T> Vector2<T>::Unit()
{
  return (T)(static_cast<T>(1.0) / Magnitude()) * (*this);
}

//______________________________________________________________________________
template <typename T>
T Vector2<T>::Dot(const Vector2<T>& other)
{
  return x * other.x + y * other.y;
}

//______________________________________________________________________________
template <typename T>
T Vector2<T>::Magnitude()
{
  return static_cast<T>(std::sqrt(x * x + y * y));
}

//______________________________________________________________________________
template <typename T>
void Rect<T>::MoveRelative(const Vector2<T>& vec)
{
  _beg += vec;
  _end += vec;
}

//______________________________________________________________________________
template <typename T>
void Rect<T>::MoveAbsolute(const Vector2<T>& vec)
{
  T width = Width();
  T height = Height();
  _beg = vec;
  _end.x = _beg.x + width;
  _end.y = _beg.y + height;
}

//______________________________________________________________________________
template <typename T>
OverlapInfo<T> Rect<T>::Overlap(const Rect<T>& other)
{
  Rect<T> intersection = GetIntersectionRect(other);
  // if the intersection left == this rectangle's left, then this is entering that rect from the left side
  // if the intersection bottom == this rectangle's bottom, then entering from above
  // return the vector of perterbation through the other rect
  OverlapInfo<T> info;
  info.collisionSides = CollisionSide::NONE;
  info.numCollisionSides = 0;

  if (!(intersection.Beg().x == _beg.x && intersection.End().x == _end.x))
  {
    if (intersection.Beg().x == _beg.x)
    {
      info.collisionSides |= CollisionSide::LEFT;
      info.numCollisionSides++;
    }
    else if (intersection.End().x == _end.x)
    {
      info.collisionSides |= CollisionSide::RIGHT;
      info.numCollisionSides++;
    }
  }
  if (!(intersection.Beg().y == _beg.y && intersection.End().y == _end.y))
  {
    if (intersection.End().y == _end.y)
    {
      info.collisionSides |= CollisionSide::DOWN;
      info.numCollisionSides++;
    }
    else if (intersection.Beg().y == _beg.y)
    {
      info.collisionSides |= CollisionSide::UP;
      info.numCollisionSides++;
    }
  }

  info.amount = Vector2<T>(intersection.Beg().x == _beg.x ? -intersection.Width() : intersection.Width(),
    intersection.End().y == _end.y ? intersection.Height() : -intersection.Height());

  return info;
}

//______________________________________________________________________________
template <typename T>
bool Rect<T>::Collides(const Rect<T>& other)
{
  return !(_end.x <= other.Beg().x || _end.y <= other.Beg().y ||
    _beg.x >= other.End().x || _beg.y >= other.End().y);
}

//______________________________________________________________________________
template <typename T>
bool Rect<T>::Collides(const Vector2<T>& point)
{
  return point.x <= _end.x && point.x >= _beg.x && point.y <= _end.y && point.y >= _beg.y;
}

//______________________________________________________________________________
template <typename T>
Rect<T> Rect<T>::GetIntersectionRect(const Rect<T>& other)
{
  T leftX = std::max(_beg.x, other.Beg().x);
  T rightX = std::min(_end.x, other.End().x);
  T topY = std::max(_beg.y, other.Beg().y);
  T bottomY = std::min(_end.y, other.End().y);

  Rect<T> intersection(leftX, topY, rightX, bottomY);
  return intersection;
}

template class Vector2<float>;
template class Vector2<double>;
template class Vector2<int>;
template class Rect<float>;
template class Rect<double>;
