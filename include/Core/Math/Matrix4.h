#pragma once
#include "Core/Math/Matrix.h"
#if defined(_WIN32)
# define M_PI           3.14159265358979323846  /* pi */
#endif

//template <> Matrix<float, 4> Matrix<float, 4>::operator*(const Matrix<float, 4>&);

typedef Matrix<float, 4> Matrix4F;

class Mat4
{
public:
  static Matrix4F Create(const float m11, const float m12, const float m13, const float m14,
    const float m21, const float m22, const float m23, const float m24,
    const float m31, const float m32, const float m33, const float m34,
    const float m41, const float m42, const float m43, const float m44);

  static Matrix4F RotationXAxis(float radians);
  static Matrix4F RotationYAxis(float radians);
  static Matrix4F RotationZAxis(float radians);
  static Matrix4F Scale(float x, float y, float z);
  static Matrix4F Translation(float x, float y, float z);

  static Vector3<float> GetPosition(const Matrix4F& matrix);

  static const Matrix4F RotateX90;
  static const Matrix4F RotateXN90;
  static const Matrix4F RotateY90;
  static const Matrix4F RotateYN90;
  static const Matrix4F RotateZ90;
  static const Matrix4F RotateZN90;
  static const Matrix4F RotateZ180;

  // helper for passing matrix to OpenGL context
  static void toMat4(const Matrix4F& mat, float* array);

};
