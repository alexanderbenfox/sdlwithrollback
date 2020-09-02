#pragma once
#include <cmath>

//______________________________________________________________________________
template <typename T>
struct Vector3
{
  Vector3<T>() : x(0), y(0), z(0) {}
  Vector3<T>(T x, T y, T z) : x(x), y(y), z(z) {}

  //! Data storage
  T x;
  T y;
  T z;

  //! Arithmetic operator overloads
  Vector3<T> operator+(const Vector3<T>& other) const { return Vector3<T>(x + other.x, y + other.y, z + other.z); }
  Vector3<T> operator-(const Vector3<T>& other) const { return Vector3<T>(x - other.x, y - other.y, z- other.z); }
  Vector3<T> operator*(const Vector3<T>& other) const { return Vector3<T>(x * other.x, y * other.y, z * other.z); }
  Vector3<T> operator/(const Vector3<T>& other) const { return Vector3<T>(x / other.x, y / other.y, z / other.z); }
  Vector3<T>& operator+=(const Vector3<T>& other) { x += other.x; y += other.y; z += other.z; return *this; }
  Vector3<T>& operator-=(const Vector3<T>& other) { x -= other.x; y -= other.y; z -= other.z; return *this; }
  Vector3<T>& operator*=(const Vector3<T>& other) { x *= other.x; y *= other.y; z *= other.z; return *this; }
  Vector3<T>& operator/=(const Vector3<T>& other) { x /= other.x; y /= other.y; z /= other.z; return *this; }

  //! Scalar arithmetic
  Vector3<T>& operator*=(T s) { x *= s; y *= s; z *= s; return *this; }
  Vector3<T>& operator/=(T s) { x /= s; y /= s; z /= s; return *this; }

  //! Negation
  Vector3<T> operator-() const { return Vector3<T>(-x, -y, -z); }

  //! Equality checks
  bool operator==(const Vector3<T>& other) const { return x == other.x && y == other.y && z == other.z; }
  bool operator!=(const Vector3<T>& other) const { return x != other.x || y != other.y || z != other.z; }

  //! Casting operator for other types of vectors (numerical only)
  template <typename U>
  operator Vector3<U>() const { return Vector3<U>((U)x, (U)y, (U)z); }

  //! Gets the normalized vector
  Vector3<T> Unit();
  //! Gets the dot products
  T Dot(const Vector3<T>& other);
  //! Gets the magnitude of the vector
  T Magnitude();

  static Vector3<T> Zero;
};

//______________________________________________________________________________
//! Scalar arithmetic
template <typename T> Vector3<T> operator*(T s, const Vector3<T>& vec) { return Vector3<T>(s * vec.x, s * vec.y, s * vec.z); }
template <typename T> Vector3<T> operator/(T s, const Vector3<T>& vec) { return Vector3<T>(vec.x / s, vec.y / s, vec.y / s); }
template <typename T> Vector3<T> operator*(const Vector3<T>& vec, T s) { return Vector3<T>(s * vec.x, s * vec.y, s * vec.z); }
template <typename T> Vector3<T> operator/(const Vector3<T>& vec, T s) { return Vector3<T>(vec.x / s, vec.y / s, vec.y / s); }

//______________________________________________________________________________
template <typename T>
inline Vector3<T> Vector3<T>::Unit()
{
  return (T)(static_cast<T>(1.0) / Magnitude()) * (*this);
}

//______________________________________________________________________________
template <typename T>
inline T Vector3<T>::Dot(const Vector3<T>& other)
{
  return x * other.x + y * other.y + z * other.z;
}

//______________________________________________________________________________
template <typename T>
inline T Vector3<T>::Magnitude()
{
  return static_cast<T>(std::sqrt(x * x + y * y + z * z));
}

//______________________________________________________________________________
template <typename T>
inline Vector3<T> Vector3<T>::Zero = Vector3<T>(static_cast<T>(0), static_cast<T>(0), static_cast<T>(0));
