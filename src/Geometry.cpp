#include "Geometry.h"
#include <cmath>

//______________________________________________________________________________
template <typename T>
Vector2<T> Vector2<T>::Unit()
{
  return (static_cast<T>(1.0) / Magnitude()) * (*this);
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
  return std::sqrt(x * x + y * y);
}

//______________________________________________________________________________
template <typename T>
void Rect<T>::Move(const Vector2<T>& vec)
{
  _beg += vec;
  _end += vec;
}

//______________________________________________________________________________
template <typename T>
Vector2<T> Rect<T>::Overlap(const Rect<T>& other, Vector2<T> incidentVector)
{
  Rect<T> intersection = GetIntersectionRect(other);

  bool resolveOnX = intersection.Width() < intersection.Height();
  if (resolveOnX)
    return Vector2<T>(intersection.Beg().x == _beg.x ? -intersection.Width() : intersection.Width(), 0.0f);
  else
    return Vector2<T>(0.0f, intersection.End().y == _end.y ? intersection.Height() : -intersection.Height());
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
template class Rect<float>;
template class Rect<double>;
