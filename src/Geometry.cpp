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
  Vector2<T> overlap;
  if (incidentVector.x > 0)
    overlap.x = _end.x - other.Beg().x;
  else
    overlap.x = other.End().x - _beg.x;
  if (incidentVector.y > 0)
    overlap.y = _end.y - other.Beg().y;
  else
    overlap.y = other.End().y - _beg.y;

  auto unit = incidentVector.Unit();
  overlap *= unit;
  return overlap;
}

//______________________________________________________________________________
template <typename T>
bool Rect<T>::Collides(const Rect<T>& other)
{
  return Collides(other.Beg()) || Collides(other.End()) || Collides(Vector2<float>(other.Beg().x, other.End().y)) || Collides(Vector2<float>(other.End().x, other.Beg().y));
}

//______________________________________________________________________________
template <typename T>
bool Rect<T>::Collides(const Vector2<T>& point)
{
  return point.x <= _end.x && point.x >= _beg.x && point.y <= _end.y && point.y >= _beg.y;
}

template class Vector2<float>;
template class Rect<float>;
