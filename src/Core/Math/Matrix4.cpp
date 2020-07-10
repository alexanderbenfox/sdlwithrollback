#include "Core/Math/Matrix4.h"

/*template <> Matrix<float, 4> Matrix<float, 4>::operator*(const Matrix<float, 4>& matrix2)
{
  Matrix result;
  result(0, 0) = operator()(0, 0) * matrix2(0, 0) + operator()(0, 1) * matrix2(1, 0) + operator()(0, 2) * matrix2(2, 0) + operator()(0, 3) * matrix2(3, 0);
  result(0, 1) = operator()(0, 0) * matrix2(0, 1) + operator()(0, 1) * matrix2(1, 1) + operator()(0, 2) * matrix2(2, 1) + operator()(0, 3) * matrix2(3, 2);
  result(0, 2) = operator()(0, 0) * matrix2(0, 2) + operator()(0, 1) * matrix2(1, 2) + operator()(0, 2) * matrix2(2, 2) + operator()(0, 3) * matrix2(3, 3);
  result(0, 3) = operator()(0, 0) * matrix2(0, 3) + operator()(0, 1) * matrix2(1, 3) + operator()(0, 2) * matrix2(2, 3) + operator()(0, 3) * matrix2(3, 4);

  result(1, 0) = operator()(1, 0) * matrix2(0, 0) + operator()(1, 1) * matrix2(1, 0) + operator()(1, 2) * matrix2(2, 0) + operator()(1, 3) * matrix2(3, 0);
  result(1, 1) = operator()(1, 0) * matrix2(0, 1) + operator()(1, 1) * matrix2(1, 1) + operator()(1, 2) * matrix2(2, 1) + operator()(1, 3) * matrix2(3, 1);
  result(1, 2) = operator()(1, 0) * matrix2(0, 2) + operator()(1, 1) * matrix2(1, 2) + operator()(1, 2) * matrix2(2, 2) + operator()(1, 3) * matrix2(3, 2);
  result(1, 3) = operator()(1, 0) * matrix2(0, 3) + operator()(1, 1) * matrix2(1, 3) + operator()(1, 2) * matrix2(2, 3) + operator()(1, 3) * matrix2(3, 3);

  result(2, 0) = operator()(2, 0) * matrix2(0, 0) + operator()(2, 1) * matrix2(1, 0) + operator()(2, 2) * matrix2(2, 0) + operator()(2, 3) * matrix2(3, 0);
  result(2, 1) = operator()(2, 0) * matrix2(0, 1) + operator()(2, 1) * matrix2(1, 1) + operator()(2, 2) * matrix2(2, 1) + operator()(2, 3) * matrix2(3, 1);
  result(2, 2) = operator()(2, 0) * matrix2(0, 2) + operator()(2, 1) * matrix2(1, 2) + operator()(2, 2) * matrix2(2, 2) + operator()(2, 3) * matrix2(3, 2);
  result(2, 3) = operator()(2, 0) * matrix2(0, 3) + operator()(2, 1) * matrix2(1, 3) + operator()(2, 2) * matrix2(2, 3) + operator()(2, 3) * matrix2(3, 3);

  result(3, 0) = operator()(3, 0) * matrix2(0, 0) + operator()(3, 1) * matrix2(1, 0) + operator()(3, 2) * matrix2(2, 0) + operator()(3, 3) * matrix2(3, 0);
  result(3, 1) = operator()(3, 0) * matrix2(0, 1) + operator()(3, 1) * matrix2(1, 1) + operator()(3, 2) * matrix2(2, 1) + operator()(3, 3) * matrix2(3, 1);
  result(3, 2) = operator()(3, 0) * matrix2(0, 2) + operator()(3, 1) * matrix2(1, 2) + operator()(3, 2) * matrix2(2, 2) + operator()(3, 3) * matrix2(3, 2);
  result(3, 3) = operator()(3, 0) * matrix2(0, 3) + operator()(3, 1) * matrix2(1, 3) + operator()(3, 2) * matrix2(2, 3) + operator()(3, 3) * matrix2(3, 3);
  return result;
}*/

Matrix4F Mat4::Create(const float m11, const float m12, const float m13, const float m14,
  const float m21, const float m22, const float m23, const float m24,
  const float m31, const float m32, const float m33, const float m34,
  const float m41, const float m42, const float m43, const float m44)
{
  Matrix4F matrix;
  matrix(0) = m11;
  matrix(1) = m12;
  matrix(2) = m13;
  matrix(3) = m14;
  matrix(4) = m21;
  matrix(5) = m22;
  matrix(6) = m23;
  matrix(7) = m24;
  matrix(8) = m31;
  matrix(9) = m32;
  matrix(10) = m33;
  matrix(11) = m34;
  matrix(12) = m41;
  matrix(13) = m42;
  matrix(14) = m43;
  matrix(15) = m44;
  return matrix;
}

Matrix4F Mat4::RotationZAxis(float radians)
{
  Matrix4F rotationMatrix = Matrix4F::Identity();

  rotationMatrix[0][0] = std::cosf(radians);
  rotationMatrix[1][0] = std::sinf(radians);
  rotationMatrix[0][1] = -rotationMatrix[1][0];
  rotationMatrix[1][1] = rotationMatrix[0][0];

  return rotationMatrix;
}

Matrix4F Mat4::Scale(float x, float y, float z)
{
  Matrix4F scaleMatrix = Matrix4F::Identity();

  scaleMatrix[0][0] = x;
  scaleMatrix[1][1] = y;
  scaleMatrix[3][3] = z;

  return scaleMatrix;
}

Matrix4F Mat4::Translation(float x, float y, float z)
{
  Matrix4F translationMatrix = Matrix4F::Identity();

  translationMatrix[0][3] = x;
  translationMatrix[1][3] = y;
  translationMatrix[2][3] = z;

  return translationMatrix;
}

Vector3<float> Mat4::GetPosition(const Matrix4F& matrix)
{
  return Vector3<float>(matrix[0][3], matrix[1][3], matrix[2][3]);
}

void Mat4::toMat4(const Matrix4F& mat, float* array)
{
  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      array[i + j * 4] = mat[j][i];
    }
  }
}
