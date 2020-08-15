#pragma once
#include "Core/Math/Vector2.h"
#include "Core/Math/Vector3.h"

/*template <typename T, int S> struct VectorType { struct V {}; };
template <typename T> struct VectorType<T, 2> { using V = Vector2<T>; };
template <typename T> struct VectorType<T, 3> { using V = Vector3<T>; };*/

//! Same num columns and rows matrix (square matrix)
template <typename T, int S>
class Matrix
{
public:
  //using Vector = VectorType<T, S>::V;
  //!
  Matrix() = default;
  Matrix(const T& initialValue);

  //! Get Identity Matrix
  static Matrix<T, S> Identity();

  // Assignment operator
  //Matrix& operator=(const Matrix<T, S>& other);

  // comparison
  bool operator==(const Matrix<T, S>& other);

  // Matrix math operations
  Matrix operator+(const Matrix<T, S>& other);
  Matrix& operator+=(const Matrix<T, S>& other);
  Matrix operator-(const Matrix<T, S>& other);
  Matrix& operator-=(const Matrix<T, S>& other);
  Matrix operator*(const Matrix<T, S>& other);
  Matrix& operator*=(const Matrix<T, S>& other);

  //
  Matrix Transpose() const;

  // Scalar operations
  Matrix operator+(const T& s);
  Matrix operator-(const T& s);
  Matrix operator*(const T& s);
  Matrix operator/(const T& s);

  // Matrix/vector operations
  /*Vector operator*(const Vector& other);
  Vector GetDiagonal() const;*/

  // Accessors
  T* operator[](const int& row);
  T const* operator[](const int& row) const;
  T& operator()(const int& column, const int& row);
  T const& operator()(const int& column, const int& row) const;
  T& operator()(const int& index);

protected:
  //! Access to memory as to array of elements.
  T _elements[S * S];

  //! Helper for applying a single operation to all elements in the matrix
  template <typename Operator>
  void ForAll(Operator op, const T& value);

};

template <typename T, int S>
inline Matrix<T, S>::Matrix(const T& initialValue)
{
  for (int i = 0; i < S; i++)
  {
    for (int j = 0; j < S; j++)
    {
      _elements[i + j * S] = initialValue;
    }
  }
}

/*template <typename T, int S>
Matrix<T, S>& Matrix<T, S>::operator=(const Matrix<T, S>& other)
{
}*/

template <typename T, int S>
inline Matrix<T, S> Matrix<T, S>::Identity()
{
  Matrix<T, S> matrix(static_cast<T>(0));
  for (int i = 0; i < S; i++)
  {
    matrix[i][i] = static_cast<T>(1);
  }
  return matrix;
}

template <typename T, int S>
inline bool Matrix<T, S>::operator==(const Matrix<T, S>& other)
{
  for (int i = 0; i < S; i++)
  {
    for (int j = 0; j < S; j++)
    {
      const int index = i + j * S;
      if (_elements[index] != other[j][i])
        return false;
    }
  }
  return true;
}

template <typename T, int S>
inline Matrix<T, S> Matrix<T, S>::operator+(const Matrix<T, S>& other)
{
  Matrix<T, S> matrix;
  for (int i = 0; i < S; i++)
  {
    for (int j = 0; j < S; j++)
    {
      matrix[j][i] = _elements[i + j * S] + other[j][i];
    }
  }
  return matrix;
}

template <typename T, int S>
inline Matrix<T, S>& Matrix<T, S>::operator+=(const Matrix<T, S>& other)
{
  for (int i = 0; i < S; i++)
  {
    for (int j = 0; j < S; j++)
    {
      _elements[i + j * S] += other[j][i];
    }
  }
  return *this;
}

template <typename T, int S>
inline Matrix<T, S> Matrix<T, S>::operator-(const Matrix<T, S>& other)
{
  Matrix<T, S> matrix;
  for (int i = 0; i < S; i++)
  {
    for (int j = 0; j < S; j++)
    {
      matrix[j][i] = _elements[i + j * S] - other[j][i];
    }
  }
  return matrix;
}

template <typename T, int S>
inline Matrix<T, S>& Matrix<T, S>::operator-=(const Matrix<T, S>& other)
{
  for (int i = 0; i < S; i++)
  {
    for (int j = 0; j < S; j++)
    {
      _elements[i + j * S] -= other[j][i];
    }
  }
  return *this;
}

template <typename T, int S>
inline Matrix<T, S> Matrix<T, S>::operator*(const Matrix<T, S>& other)
{
  Matrix<T, S> mult(static_cast<T>(0));
  for (int i = 0; i < S; i++)
  {
    for (int j = 0; j < S; j++)
    {
      for (int k = 0; k < S; k++)
        mult[j][i] += (_elements[i + k * S] * other[j][k]);
    }
  }
  return mult;
}

template <typename T, int S>
inline Matrix<T, S>& Matrix<T, S>::operator*=(const Matrix<T, S>& other)
{
  for (int i = 0; i < S; i++)
  {
    for (int j = 0; j < S; j++)
    {
      T temp = static_cast<T>(0);
      for (int k = 0; k < S; k++)
         temp += (_elements[i + k * S] * other[j][k]);
      _elements[i + j * S] = temp;
    }
  }
  return *this;
}

template <typename T, int S>
inline Matrix<T, S> Matrix<T, S>::Transpose() const
{
  Matrix<T, S> transpose(static_cast<T>(0));
  for (int i = 0; i < S; i++)
  {
    for (int j = 0; j < S; j++)
    {
      transpose[j][i] = _elements[j + i * S];
    }
  }
  return transpose;
}

template <typename T, int S>
inline Matrix<T, S> Matrix<T, S>::operator+(const T& s)
{
  Matrix<T, S> matrix = *this;
  matrix.ForAll(T::operator+, s);
  return matrix;
}

template <typename T, int S>
inline Matrix<T, S> Matrix<T, S>::operator-(const T& s)
{
  Matrix<T, S> matrix = *this;
  matrix.ForAll(T::operator-, s);
  return matrix;
}

template <typename T, int S>
inline Matrix<T, S> Matrix<T, S>::operator*(const T& s)
{
  Matrix<T, S> matrix = *this;
  matrix.ForAll([](const T& a, const T& b) { return a * b; }, s);
  return matrix;
}

template <typename T, int S>
inline Matrix<T, S> Matrix<T, S>::operator/(const T& s)
{
  Matrix<T, S> matrix = *this;
  matrix.ForAll(T::operator/, s);
  return matrix;
}

/*template <typename T, int S>
Matrix<T, S>::Vector Matrix<T, S>::operator*(const Vector& other)
{

}

template <typename T, int S>
Matrix<T, S>::Vector Matrix<T, S>::GetDiagonal() const
{

}*/

template <typename T, int S>
inline T* Matrix<T, S>::operator[](const int& row)
{
  return _elements + (row * S);
}

template <typename T, int S>
inline T const* Matrix<T, S>::operator[](const int& row) const
{
  return _elements + (row * S);
}

template <typename T, int S>
inline T& Matrix<T, S>::operator()(const int& column, const int& row)
{
  return _elements[column + row * S];
}

template <typename T, int S>
inline T const& Matrix<T, S>::operator()(const int& column, const int& row) const
{
  return _elements[column + row * S];
}

template <typename T, int S>
inline T& Matrix<T, S>::operator()(const int& index)
{
  return _elements[index];
}

template <typename T, int S>
template <typename Operator>
inline void Matrix<T, S>::ForAll(Operator op, const T& value)
{
  for (int i = 0; i < S; i++)
  {
    for (int j = 0; j < S; j++)
    {
      _elements[i + j * S] = op(_elements[i + j * S], value);
    }
  }
}
