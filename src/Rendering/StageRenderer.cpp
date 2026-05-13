#include "Rendering/StageRenderer.h"
#include "Components/Camera.h"
#include "Core/Math/Matrix4.h"
#include "Globals.h"
#include <bx/math.h>
#include <fstream>

// View ID for 3D stage rendering
static constexpr uint16_t VIEW_3D = 0;

bgfx::VertexLayout MeshVertex::layout;

void MeshVertex::Init()
{
  layout
    .begin()
    .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
    .add(bgfx::Attrib::Normal,   3, bgfx::AttribType::Float)
    .add(bgfx::Attrib::Color0,   4, bgfx::AttribType::Uint8, true)
    .end();
}

static bgfx::ShaderHandle LoadShader(const char* path)
{
  std::ifstream file(path, std::ios::binary | std::ios::ate);
  if (!file.is_open())
    return BGFX_INVALID_HANDLE;

  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  const bgfx::Memory* mem = bgfx::alloc(uint32_t(size) + 1);
  file.read(reinterpret_cast<char*>(mem->data), size);
  mem->data[size] = '\0';

  return bgfx::createShader(mem);
}

static uint32_t PackColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
{
  // bgfx Metal uses UChar4Normalized: reads bytes in order as r,g,b,a
  // Pack as uint32 with R in LSB (ABGR layout) → memory bytes [R,G,B,A]
  return (uint32_t(a) << 24) | (uint32_t(b) << 16) | (uint32_t(g) << 8) | uint32_t(r);
}

void StageRenderer::Init()
{
  MeshVertex::Init();

  bgfx::ShaderHandle vs = LoadShader("resources/shaders/metal/vs_color3d.bin");
  bgfx::ShaderHandle fs = LoadShader("resources/shaders/metal/fs_color3d.bin");
  _meshProgram = bgfx::createProgram(vs, fs, true);

  CreateFloor();
  CreateWalls();
  CreateCube();
  CreatePyramid();
}

void StageRenderer::CreateFloor()
{
  // Floor: horizontal quad at y=-1, in XZ plane
  // Matches original: stageSize(6,4), position(0,-1,0), glTranslate(0,0,3)
  const float hw = 6.0f;  // half-width in X
  const float hd = 4.0f;  // half-depth in Z
  const float y = -1.0f;
  const float zOffset = 3.0f;
  uint32_t color = PackColor(230, 230, 230);

  MeshVertex verts[] = {
    {  hw, y, -hd + zOffset,  0, 1, 0, color },
    { -hw, y, -hd + zOffset,  0, 1, 0, color },
    { -hw, y,  hd + zOffset,  0, 1, 0, color },
    {  hw, y,  hd + zOffset,  0, 1, 0, color },
  };
  uint16_t indices[] = { 0, 1, 2, 0, 2, 3 };

  _floorVB = bgfx::createVertexBuffer(bgfx::copy(verts, sizeof(verts)), MeshVertex::layout);
  _floorIB = bgfx::createIndexBuffer(bgfx::copy(indices, sizeof(indices)));
}

void StageRenderer::CreateWalls()
{
  const float hw = 6.0f;
  const float hd = 4.0f;
  const float zOffset = 3.0f;
  const float wallHeight = 8.0f;
  const float floorY = -1.0f;

  // Back wall: vertical quad at z = -hd + zOffset, facing +Z (toward camera)
  {
    uint32_t color = PackColor(153, 153, 153);
    float z = -hd + zOffset;
    MeshVertex verts[] = {
      { -hw, floorY,              z,  0, 0, 1, color },
      {  hw, floorY,              z,  0, 0, 1, color },
      {  hw, floorY + wallHeight, z,  0, 0, 1, color },
      { -hw, floorY + wallHeight, z,  0, 0, 1, color },
    };
    uint16_t indices[] = { 0, 1, 2, 0, 2, 3 };
    _backWallVB = bgfx::createVertexBuffer(bgfx::copy(verts, sizeof(verts)), MeshVertex::layout);
    _backWallIB = bgfx::createIndexBuffer(bgfx::copy(indices, sizeof(indices)));
  }

  // Left wall: vertical quad at x = -hw, facing +X
  {
    uint32_t color = PackColor(128, 128, 128);
    float x = -hw;
    MeshVertex verts[] = {
      { x, floorY,              -hd + zOffset,  1, 0, 0, color },
      { x, floorY,               hd + zOffset,  1, 0, 0, color },
      { x, floorY + wallHeight,  hd + zOffset,  1, 0, 0, color },
      { x, floorY + wallHeight, -hd + zOffset,  1, 0, 0, color },
    };
    uint16_t indices[] = { 0, 1, 2, 0, 2, 3 };
    _leftWallVB = bgfx::createVertexBuffer(bgfx::copy(verts, sizeof(verts)), MeshVertex::layout);
    _leftWallIB = bgfx::createIndexBuffer(bgfx::copy(indices, sizeof(indices)));
  }

  // Right wall: vertical quad at x = hw, facing -X
  {
    uint32_t color = PackColor(128, 128, 128);
    float x = hw;
    MeshVertex verts[] = {
      { x, floorY,               hd + zOffset, -1, 0, 0, color },
      { x, floorY,              -hd + zOffset, -1, 0, 0, color },
      { x, floorY + wallHeight, -hd + zOffset, -1, 0, 0, color },
      { x, floorY + wallHeight,  hd + zOffset, -1, 0, 0, color },
    };
    uint16_t indices[] = { 0, 1, 2, 0, 2, 3 };
    _rightWallVB = bgfx::createVertexBuffer(bgfx::copy(verts, sizeof(verts)), MeshVertex::layout);
    _rightWallIB = bgfx::createIndexBuffer(bgfx::copy(indices, sizeof(indices)));
  }
}

void StageRenderer::CreateCube()
{
  // Cube at position (1, 0, 2), scale 0.4
  const float s = 0.4f;
  const float cx = 1.0f, cy = 0.0f, cz = 2.0f;

  uint32_t colors[6] = {
    PackColor(0, 255, 0),     // top
    PackColor(255, 128, 0),   // bottom
    PackColor(255, 0, 0),     // front
    PackColor(255, 255, 0),   // back
    PackColor(0, 0, 255),     // left
    PackColor(255, 0, 255),   // right
  };

  MeshVertex verts[] = {
    // Top face (y+)
    { cx + s, cy + s, cz - s,  0, 1, 0, colors[0] },
    { cx - s, cy + s, cz - s,  0, 1, 0, colors[0] },
    { cx - s, cy + s, cz + s,  0, 1, 0, colors[0] },
    { cx + s, cy + s, cz + s,  0, 1, 0, colors[0] },
    // Bottom face (y-)
    { cx + s, cy - s, cz + s,  0,-1, 0, colors[1] },
    { cx - s, cy - s, cz + s,  0,-1, 0, colors[1] },
    { cx - s, cy - s, cz - s,  0,-1, 0, colors[1] },
    { cx + s, cy - s, cz - s,  0,-1, 0, colors[1] },
    // Front face (z+)
    { cx + s, cy + s, cz + s,  0, 0, 1, colors[2] },
    { cx - s, cy + s, cz + s,  0, 0, 1, colors[2] },
    { cx - s, cy - s, cz + s,  0, 0, 1, colors[2] },
    { cx + s, cy - s, cz + s,  0, 0, 1, colors[2] },
    // Back face (z-)
    { cx + s, cy - s, cz - s,  0, 0,-1, colors[3] },
    { cx - s, cy - s, cz - s,  0, 0,-1, colors[3] },
    { cx - s, cy + s, cz - s,  0, 0,-1, colors[3] },
    { cx + s, cy + s, cz - s,  0, 0,-1, colors[3] },
    // Left face (x-)
    { cx - s, cy + s, cz + s, -1, 0, 0, colors[4] },
    { cx - s, cy + s, cz - s, -1, 0, 0, colors[4] },
    { cx - s, cy - s, cz - s, -1, 0, 0, colors[4] },
    { cx - s, cy - s, cz + s, -1, 0, 0, colors[4] },
    // Right face (x+)
    { cx + s, cy + s, cz - s,  1, 0, 0, colors[5] },
    { cx + s, cy + s, cz + s,  1, 0, 0, colors[5] },
    { cx + s, cy - s, cz + s,  1, 0, 0, colors[5] },
    { cx + s, cy - s, cz - s,  1, 0, 0, colors[5] },
  };

  uint16_t indices[] = {
     0,  1,  2,  0,  2,  3, // top
     4,  5,  6,  4,  6,  7, // bottom
     8,  9, 10,  8, 10, 11, // front
    12, 13, 14, 12, 14, 15, // back
    16, 17, 18, 16, 18, 19, // left
    20, 21, 22, 20, 22, 23, // right
  };

  _cubeVB = bgfx::createVertexBuffer(bgfx::copy(verts, sizeof(verts)), MeshVertex::layout);
  _cubeIB = bgfx::createIndexBuffer(bgfx::copy(indices, sizeof(indices)));
}

void StageRenderer::CreatePyramid()
{
  // Pyramid at position (-1.2, 0, 2.7), scale 0.3
  const float s = 0.3f;
  const float cx = -1.2f, cy = 0.0f, cz = 2.7f;

  uint32_t c0 = PackColor(255, 0, 0);
  uint32_t c1 = PackColor(0, 255, 0);
  uint32_t c2 = PackColor(0, 0, 255);

  // 4 triangular faces (no base)
  MeshVertex verts[] = {
    // Front
    { cx,        cy + s, cz,        0, 0.5f, 0.87f, c0 },
    { cx - s,    cy - s, cz + s,    0, 0.5f, 0.87f, c1 },
    { cx + s,    cy - s, cz + s,    0, 0.5f, 0.87f, c2 },
    // Right
    { cx,        cy + s, cz,        0.87f, 0.5f, 0, c0 },
    { cx + s,    cy - s, cz + s,    0.87f, 0.5f, 0, c2 },
    { cx + s,    cy - s, cz - s,    0.87f, 0.5f, 0, c1 },
    // Back
    { cx,        cy + s, cz,        0, 0.5f,-0.87f, c0 },
    { cx + s,    cy - s, cz - s,    0, 0.5f,-0.87f, c1 },
    { cx - s,    cy - s, cz - s,    0, 0.5f,-0.87f, c2 },
    // Left
    { cx,        cy + s, cz,       -0.87f, 0.5f, 0, c0 },
    { cx - s,    cy - s, cz - s,   -0.87f, 0.5f, 0, c2 },
    { cx - s,    cy - s, cz + s,   -0.87f, 0.5f, 0, c1 },
  };

  uint16_t indices[] = {
    0, 1, 2,
    3, 4, 5,
    6, 7, 8,
    9, 10, 11,
  };

  _pyramidVB = bgfx::createVertexBuffer(bgfx::copy(verts, sizeof(verts)), MeshVertex::layout);
  _pyramidIB = bgfx::createIndexBuffer(bgfx::copy(indices, sizeof(indices)));
}

void StageRenderer::Render(Camera* camera, uint32_t viewWidth, uint32_t viewHeight)
{
  // Set up view and projection matrices
  float view[16];
  float proj[16];

  Vector3<float> camPos(0, 0, 0);
  if (camera)
    camPos = Mat4::GetPosition(camera->worldMatrix);

  bx::Vec3 eye    = { camPos.x, camPos.y, 5.0f };
  bx::Vec3 at     = { camPos.x, camPos.y, 0.0f };
  bx::Vec3 up     = { 0.0f, 1.0f, 0.0f };

  // Right-handed: forward is -Z, matching our eye(z=5) looking at at(z=0)
  bx::mtxLookAt(view, eye, at, up, bx::Handedness::Right);

  const bgfx::Caps* caps = bgfx::getCaps();
  float aspect = (viewHeight > 0) ? float(viewWidth) / float(viewHeight) : (float(m_nativeWidth) / float(m_nativeHeight));
  bx::mtxProj(proj, 54.0f, aspect, 1.0f, 1000.0f, caps->homogeneousDepth, bx::Handedness::Right);

  bgfx::setViewTransform(VIEW_3D, view, proj);

  // Identity model matrix for all static geometry
  float identity[16];
  bx::mtxIdentity(identity);

  uint64_t state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z
                 | BGFX_STATE_DEPTH_TEST_LESS
                 | BGFX_STATE_CULL_CW
                 | BGFX_STATE_MSAA;

  // Floor
  bgfx::setTransform(identity);
  bgfx::setVertexBuffer(0, _floorVB);
  bgfx::setIndexBuffer(_floorIB);
  bgfx::setState(state);
  bgfx::submit(VIEW_3D, _meshProgram);

  // Back wall
  bgfx::setTransform(identity);
  bgfx::setVertexBuffer(0, _backWallVB);
  bgfx::setIndexBuffer(_backWallIB);
  bgfx::setState(state);
  bgfx::submit(VIEW_3D, _meshProgram);

  // Left wall
  bgfx::setTransform(identity);
  bgfx::setVertexBuffer(0, _leftWallVB);
  bgfx::setIndexBuffer(_leftWallIB);
  bgfx::setState(state);
  bgfx::submit(VIEW_3D, _meshProgram);

  // Right wall
  bgfx::setTransform(identity);
  bgfx::setVertexBuffer(0, _rightWallVB);
  bgfx::setIndexBuffer(_rightWallIB);
  bgfx::setState(state);
  bgfx::submit(VIEW_3D, _meshProgram);

  // Cube
  bgfx::setTransform(identity);
  bgfx::setVertexBuffer(0, _cubeVB);
  bgfx::setIndexBuffer(_cubeIB);
  bgfx::setState(state);
  bgfx::submit(VIEW_3D, _meshProgram);

  // Pyramid
  bgfx::setTransform(identity);
  bgfx::setVertexBuffer(0, _pyramidVB);
  bgfx::setIndexBuffer(_pyramidIB);
  bgfx::setState(state);
  bgfx::submit(VIEW_3D, _meshProgram);
}

void StageRenderer::Shutdown()
{
  bgfx::destroy(_meshProgram);
  bgfx::destroy(_floorVB);
  bgfx::destroy(_floorIB);
  bgfx::destroy(_backWallVB);
  bgfx::destroy(_backWallIB);
  bgfx::destroy(_leftWallVB);
  bgfx::destroy(_leftWallIB);
  bgfx::destroy(_rightWallVB);
  bgfx::destroy(_rightWallIB);
  bgfx::destroy(_cubeVB);
  bgfx::destroy(_cubeIB);
  bgfx::destroy(_pyramidVB);
  bgfx::destroy(_pyramidIB);
}
