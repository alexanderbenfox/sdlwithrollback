#pragma once
#include <string>
#include "Core/Math/Vector3.h"
#include "Core/Math/Matrix4.h"

//! Inteface for a loadable shader
class IShader
{
public:
  virtual ~IShader() = default;

  virtual void Bind() const = 0;
  virtual void Unbind() const = 0;

	virtual void SetInt(const std::string& name, int value) = 0;
	virtual void SetIntArray(const std::string& name, int* values, uint32_t count) = 0;
	virtual void SetFloat(const std::string& name, float value) = 0;
	virtual void SetFloat3(const std::string& name, const Vector3<float>& value) = 0;
	//virtual void SetFloat4(const std::string& name, const glm::vec4& value) = 0;
	virtual void SetMat4(const std::string& name, const Matrix4<float>& value) = 0;

};
