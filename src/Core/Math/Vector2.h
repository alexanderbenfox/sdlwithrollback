#pragma once
#include <iostream>
#include <cmath>

//______________________________________________________________________________
template <typename T>
struct Vector2
{
  typedef T DataType;

  Vector2<T>() : x(0), y(0) {}
  Vector2<T>(T x, T y) : x(x), y(y) {}

  //! Data storage
  T x;
  T y;

  //! Arithmetic operator overloads
  Vector2<T> operator+(const Vector2<T>& other) const { return Vector2<T>(x + other.x, y + other.y); }
  Vector2<T> operator-(const Vector2<T>& other) const { return Vector2<T>(x - other.x, y - other.y); }
  Vector2<T> operator*(const Vector2<T>& other) const { return Vector2<T>(x * other.x, y * other.y); }
  Vector2<T> operator/(const Vector2<T>& other) const { return Vector2<T>(x / other.x, y / other.y); }
  Vector2<T>& operator+=(const Vector2<T>& other) { x += other.x; y += other.y; return *this; }
  Vector2<T>& operator-=(const Vector2<T>& other) { x -= other.x; y -= other.y; return *this; }
  Vector2<T>& operator*=(const Vector2<T>& other) { x *= other.x; y *= other.y; return *this; }
  Vector2<T>& operator/=(const Vector2<T>& other) { x /= other.x; y /= other.y; return *this; }

  //! Scalar arithmetic
  Vector2<T>& operator*=(T s) { x *= s; y *= s; return *this; }
  Vector2<T>& operator/=(T s) { x /= s; y /= s; return *this; }
 
  //! Negation
  Vector2<T> operator-() const { return Vector2<T>(-x, -y); }
  //! Equality checks
  bool operator==(const Vector2<T>& other) const { return x == other.x && y == other.y; }
  bool operator!=(const Vector2<T>& other) const { return x != other.x || y != other.y; }

  //! Casting operator for other types of vectors (numerical only)
  template <typename U>
  operator Vector2<U>() const { return Vector2<U>((U)x, (U)y); }

  //! Gets the normalized vector
  Vector2<T> Unit() const;
  //! Gets the dot products
  T Dot(const Vector2<T>& other) const;
  //! Gets the magnitude of the vector
  T Magnitude() const;

  static Vector2<T> Zero;

  //! Stream stuff
  template <typename U>
  friend std::ostream& operator<<(std::ostream& os, const Vector2<U>& vector);
  template <typename U>
  friend std::istream& operator>>(std::istream& is, Vector2<U>& vector);

};

//______________________________________________________________________________
//! Scalar arithmetic
template <typename T> Vector2<T> operator*(T s, const Vector2<T>& vec) { return Vector2<T>(s * vec.x, s * vec.y); }
template <typename T> Vector2<T> operator/(T s, const Vector2<T>& vec) { return Vector2<T>(vec.x / s, vec.y / s); }
template <typename T> Vector2<T> operator*(const Vector2<T>& vec, T s) { return Vector2<T>(s * vec.x, s * vec.y); }
template <typename T> Vector2<T> operator/(const Vector2<T>& vec, T s) { return Vector2<T>(vec.x / s, vec.y / s); }

//______________________________________________________________________________
template <typename T>
inline Vector2<T> Vector2<T>::Unit() const
{
  return (T)(static_cast<T>(1.0) / Magnitude()) * (*this);
}

//______________________________________________________________________________
template <typename T>
inline T Vector2<T>::Dot(const Vector2<T>& other) const
{
  return x * other.x + y * other.y;
}

//______________________________________________________________________________
template <typename T>
inline T Vector2<T>::Magnitude() const
{
  return static_cast<T>(std::sqrt(x * x + y * y));
}

//______________________________________________________________________________
template <typename T>
inline Vector2<T> Vector2<T>::Zero = Vector2<T>(static_cast<T>(0), static_cast<T>(0));

//______________________________________________________________________________
template <typename U>
std::ostream& operator<<(std::ostream& os, const Vector2<U>& vector)
{
  // use read instead of << when writing literals so they can be saved in stringstream buffer
  os.write((const char*)&vector.x, sizeof(U));
  os.write((const char*)&vector.y, sizeof(U));
  return os;
}

//______________________________________________________________________________
template <typename U>
std::istream& operator>>(std::istream& is, Vector2<U>& vector)
{
  // use read instead of >> when reading literals from stringstream buffer
  is.read((char*)&vector.x, sizeof(U));
  is.read((char*)&vector.y, sizeof(U));
  return is;
}
