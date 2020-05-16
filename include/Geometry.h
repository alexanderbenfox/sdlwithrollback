#pragma once
#include <algorithm>
#include <iostream>

enum class CollisionSide : unsigned char
{
  NONE = 0x00,
  UP = 0x01,
  DOWN = 0x02,
  RIGHT = 0x04,
  LEFT = 0x08
};

//______________________________________________________________________________
void operator|=(CollisionSide& the, CollisionSide other);
//______________________________________________________________________________
CollisionSide operator|(CollisionSide a, CollisionSide b);
//______________________________________________________________________________
void operator&=(CollisionSide& the, CollisionSide other);
//______________________________________________________________________________
CollisionSide operator&(CollisionSide a, CollisionSide b);
//______________________________________________________________________________
CollisionSide operator~(CollisionSide og);
//______________________________________________________________________________
static bool HasState(const CollisionSide& state, CollisionSide other) { return (state & other) == other; }
//! Returns true if states share at least 1 common
static bool HasUnion(const CollisionSide& state, CollisionSide other) { return (state & other) != CollisionSide::NONE; }


//______________________________________________________________________________
template <typename T>
class Vector2
{
public:
  Vector2<T>() : x(0), y(0) {}
  Vector2<T>(T x, T y) : x(x), y(y) {}

  T x;
  T y;

  void operator+=(const Vector2<T>& other) { x += other.x; y += other.y; }
  void operator-=(const Vector2<T>& other) { x -= other.x; y -= other.y; }
  void operator*=(const Vector2<T>& other) { x *= other.x; y *= other.y; }
  bool operator==(const Vector2<T>& other) const { return x == other.x && y == other.y; }

  //! Casting operator for other types of vectors (numerical only)
  template <typename U>
  operator Vector2<U>() const { return Vector2<U>((U)x, (U)y); }

  friend Vector2<T> operator+(const Vector2<T>& a, const Vector2<T>& b)
  {
    return Vector2<T>(a.x + b.x, a.y + b.y);
  }
  friend Vector2<T> operator-(const Vector2<T>& a, const Vector2<T>& b)
  {
    return Vector2<T>(a.x - b.x, a.y - b.y);
  }

  Vector2<T> Unit();
  T Dot(const Vector2<T>& other);
  T Magnitude();

  friend std::ostream& operator<<(std::ostream& os, const Vector2& vec)
  {
    os << vec.x;
    os << vec.y;
    return os;
  }

  friend std::istream& operator>>(std::istream& is, Vector2& vec)
  {
    is >> vec.x;
    is >> vec.y ;
    return is;
  }

  static Vector2<T> Zero;
};

//______________________________________________________________________________
template <typename T>
inline Vector2<T> Vector2<T>::Zero = Vector2<T>(static_cast<T>(0), static_cast<T>(0));

template <typename T> Vector2<T> operator+(const Vector2<T>& lhs, const Vector2<T>& rhs);
template <typename T> Vector2<T> operator*(const Vector2<T>& lhs, T rhs);
template <typename T> Vector2<T> operator*(T lhs, const Vector2<T>& rhs);
template <typename T, typename U> Vector2<T> operator*(const Vector2<T>& lhs, U rhs);

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
template <typename T>
class Rect
{
public:
  Rect() : _beg(0, 0), _end(0, 0) {}
  Rect(Vector2<T> beg, Vector2<T> end) : _beg(beg), _end(end) {}
  Rect(T xMin, T yMin, T xMax, T yMax) : _beg(xMin, yMin), _end(xMax, yMax) {}

  T Width() const { return _end.x - _beg.x; }
  T Height() const { return _end.y - _beg.y; }
  T HalfWidth() const { return Width() / (T)2.0; }
  T HalfHeight() const { return Height() / (T)2.0; }

  void MoveRelative(const Vector2<T>& vec);
  void MoveAbsolute(const Vector2<T>& vec);

  Vector2<T> GetCenter() const { return Vector2<T>(_beg.x + (_end.x - _beg.x) / 2.0, _beg.y + (_end.y - _beg.y) / 2.0); }
  void CenterOnPoint(const Vector2<T> center)
  {
    T halfW = HalfWidth();
    T halfH = HalfHeight();
    _beg = Vector2<T>(center.x - halfW, center.y - halfH);
    _end = Vector2<T>(center.x + halfW, center.y + halfH);
  }

  void Scale(Vector2<T> oldScale, Vector2<T> newScale)
  {
    Vector2<T> scaler(Width() * (newScale.x - oldScale.x) / (T)2.0, Height() * (newScale.y - oldScale.y) / (T)2.0);
    _beg -= scaler;
    _end += scaler;
  }

  //template <typename U>
  //Vector2<T> Overlap(const U& other, Vector2<T> incidentVector);

  //template <typename U>
  //bool Collides(const U& other);

  OverlapInfo<T> Overlap(const Rect<T>& other);
  bool Collides(const Rect<T>& other);
  bool Collides(const Vector2<T>& other);

  Rect<T> GetIntersectionRect(const Rect<T>& other);

  const Vector2<T>& Beg() const { return _beg; }
  const Vector2<T>& End() const { return _end; }

  const T Area() { return Width() * Height(); }

  friend std::ostream& operator<<(std::ostream& os, const Rect& rect)
  {
    os << rect._beg;
    os << rect._end;
    return os;
  }

  friend std::istream& operator>>(std::istream& is, Rect& rect)
  {
    is >> rect._beg;
    is >> rect._end;
    return is;
  }


private:
  Vector2<T> _beg;
  Vector2<T> _end;

};

//______________________________________________________________________________
template <typename T>
inline Vector2<T> operator+(const Vector2<T>& lhs, const Vector2<T>& rhs) { return Vector2<T>(lhs.x + rhs.x, lhs.y + rhs.y); }

//______________________________________________________________________________
template <typename T>
inline Vector2<T> operator*(const Vector2<T>& lhs, T rhs) { return Vector2<T>(lhs.x * rhs, lhs.y * rhs); }

//______________________________________________________________________________
template <typename T>
inline Vector2<T> operator*(T lhs, const Vector2<T>& rhs) { return Vector2<T>(lhs * rhs.x, lhs * rhs.y); }

//______________________________________________________________________________
template <typename T, typename U>
inline Vector2<T> operator*(const Vector2<T>& lhs, U rhs) { return Vector2<T>(static_cast<T>(static_cast<U>(lhs.x) * rhs), static_cast<T>(static_cast<U>(lhs.y) * rhs)); }