#pragma once
#include <bgfx/bgfx.h>
#include "Core/Math/Vector3.h"

class Camera;

struct MeshVertex
{
  float x, y, z;
  float nx, ny, nz;
  uint32_t abgr;

  static bgfx::VertexLayout layout;
  static void Init();
};

class StageRenderer
{
public:
  void Init();
  void Render(Camera* camera, uint32_t viewWidth, uint32_t viewHeight);
  void Shutdown();

private:
  void CreateFloor();
  void CreateWalls();
  void CreateCube();
  void CreatePyramid();

  bgfx::ProgramHandle _meshProgram;

  bgfx::VertexBufferHandle _floorVB;
  bgfx::IndexBufferHandle _floorIB;

  bgfx::VertexBufferHandle _backWallVB;
  bgfx::IndexBufferHandle _backWallIB;

  bgfx::VertexBufferHandle _leftWallVB;
  bgfx::IndexBufferHandle _leftWallIB;

  bgfx::VertexBufferHandle _rightWallVB;
  bgfx::IndexBufferHandle _rightWallIB;

  bgfx::VertexBufferHandle _cubeVB;
  bgfx::IndexBufferHandle _cubeIB;

  bgfx::VertexBufferHandle _pyramidVB;
  bgfx::IndexBufferHandle _pyramidIB;
};
