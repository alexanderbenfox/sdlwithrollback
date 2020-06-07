#pragma once
#include "Core/Math/Vector2.h"

#include <algorithm>
#include <cmath>

//______________________________________________________________________________
template <typename T>
struct Rect
{
  typedef Vector2<T> PointType;

  Vector2<T> beg;
  Vector2<T> end;

  Rect() : beg(0, 0), end(0, 0) {}
  Rect(Vector2<T> beg, Vector2<T> end) : beg(beg), end(end) {}
  Rect(T xMin, T yMin, T xMax, T yMax) : beg(xMin, yMin), end(xMax, yMax) {}

  T Width() const { return end.x - beg.x; }
  T Height() const { return end.y - beg.y; }
  T HalfWidth() const { return Width() / (T)2.0; }
  T HalfHeight() const { return Height() / (T)2.0; }
  const T Area() const { return Width() * Height(); }
  PointType GetCenter() const { return Vector2<T>(beg.x + (end.x - beg.x) / 2.0, beg.y + (end.y - beg.y) / 2.0); }

  // Check if point is inside rectangle
  bool Intersects(const PointType& other) const;
  // Check if other rectangle intersects with this one
  bool Intersects(const Rect<T>& other) const;
  // Get intersecting rectangle (all sides inclusive)
  Rect<T> GetIntersection(const Rect<T>& other) const;
  

  //______________________________________________________________________________
  //! Modification functions 

  // Extend rectangle to include specified point
  void Include(const PointType& point);
  // Extend rectangle to include another rect
  void Include(const Rect<T>& other) { Include(other.beg); Include(other.end); }

  // Move the vector amount from the rectangle's current position
  void MoveRelative(const PointType& vec);
  // Move the rectangle's begin to the provided position
  void MoveAbsolute(const PointType& position);

  //! Move rectangle so that it centers on the provided point
  void CenterOnPoint(const PointType& center);
  //! Scale rectangle from one scale to another
  void Scale(Vector2<T> oldScale, Vector2<T> newScale);

  //______________________________________________________________________________
  //! Stream stff
  template <typename U>
  friend std::ostream& operator<<(std::ostream& os, const Rect<U>& rect);
  template <typename U>
  friend std::istream& operator>>(std::istream& is, Rect<U>& rect);

};

//______________________________________________________________________________
template <typename T>
inline bool Rect<T>::Intersects(const PointType& point) const
{
  return point.x <= end.x && point.x >= beg.x && point.y <= end.y && point.y >= beg.y;
}

//______________________________________________________________________________
template <typename T>
inline bool Rect<T>::Intersects(const Rect<T>& other) const
{
  return !(end.x <= other.beg.x || end.y <= other.beg.y ||
    beg.x >= other.end.x || beg.y >= other.end.y);
}

//______________________________________________________________________________
template <typename T>
inline Rect<T> Rect<T>::GetIntersection(const Rect<T>& other) const
{
  T leftX = std::max(beg.x, other.beg.x);
  T rightX = std::min(end.x, other.end.x);
  T topY = std::max(beg.y, other.beg.y);
  T bottomY = std::min(end.y, other.end.y);

  Rect<T> intersection(leftX, topY, rightX, bottomY);
  return intersection;
}

//______________________________________________________________________________
template <typename T>
inline void Rect<T>::Include(const PointType& point)
{
  auto saturateMin = [](T& a, const T& max) { if (a > max) a = max; };
  auto saturateMax = [](T& a, const T& min) { if (a < min) a = min; };
  saturateMin(beg.x, point.x);
  saturateMin(beg.y, point.y);
  saturateMax(end.x, point.x);
  saturateMax(end.y, point.y);
}

//______________________________________________________________________________
template <typename T>
inline void Rect<T>::MoveRelative(const PointType& vec)
{
  beg += vec;
  end += vec;
}

//______________________________________________________________________________
template <typename T>
inline void Rect<T>::MoveAbsolute(const PointType& position)
{
  T width = Width();
  T height = Height();
  beg = position;
  end.x = beg.x + width;
  end.y = beg.y + height;
}

//______________________________________________________________________________
template <typename T>
inline void Rect<T>::CenterOnPoint(const PointType& center)
{
  T halfW = HalfWidth();
  T halfH = HalfHeight();
  beg = Vector2<T>(center.x - halfW, center.y - halfH);
  end = Vector2<T>(center.x + halfW, center.y + halfH);
}

//______________________________________________________________________________
template <typename T>
inline void Rect<T>::Scale(Vector2<T> oldScale, Vector2<T> newScale)
{
  Vector2<T> scaler(Width() * (newScale.x - oldScale.x) / (T)2.0, Height() * (newScale.y - oldScale.y) / (T)2.0);
  beg -= scaler;
  end += scaler;
}

//______________________________________________________________________________
template <typename U>
std::ostream& operator<<(std::ostream& os, const Rect<U>& rect)
{
  os << rect.beg;
  os << rect.end;
  return os;
}

//______________________________________________________________________________
template <typename U>
std::istream& operator>>(std::istream& is, Rect<U>& rect)
{
  is >> rect.beg;
  is >> rect.end;
  return is;
}