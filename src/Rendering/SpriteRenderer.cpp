#include "Rendering/SpriteRenderer.h"
#include "Rendering/BgfxTexture.h"
#include <bx/math.h>
#include <fstream>
#include <vector>

bgfx::VertexLayout SpriteVertex::layout;

void SpriteVertex::Init()
{
  layout
    .begin()
    .add(bgfx::Attrib::Position,  3, bgfx::AttribType::Float)
    .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
    .add(bgfx::Attrib::Color0,    4, bgfx::AttribType::Uint8, true)
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

void SpriteRenderer::Init()
{
  SpriteVertex::Init();

  bgfx::ShaderHandle vsSprite = LoadShader("resources/shaders/metal/vs_sprite.bin");
  bgfx::ShaderHandle fsSprite = LoadShader("resources/shaders/metal/fs_sprite.bin");
  _spriteProgram = bgfx::createProgram(vsSprite, fsSprite, true);

  // Color program: same vertex shader, flat-color fragment (no texture sampling)
  bgfx::ShaderHandle vsColor = LoadShader("resources/shaders/metal/vs_sprite.bin");
  bgfx::ShaderHandle fsColor = LoadShader("resources/shaders/metal/fs_color.bin");
  _colorProgram = bgfx::createProgram(vsColor, fsColor, true);

  _texUniform = bgfx::createUniform("s_texture", bgfx::UniformType::Sampler);
}

void SpriteRenderer::Shutdown()
{
  bgfx::destroy(_spriteProgram);
  bgfx::destroy(_colorProgram);
  bgfx::destroy(_texUniform);
}

static uint32_t PackColor(Color c)
{
  // bgfx Metal uses UChar4Normalized: reads bytes in order as r,g,b,a
  // Pack as uint32 with R in LSB (ABGR layout) → memory bytes [R,G,B,A]
  return (uint32_t(c.a) << 24) | (uint32_t(c.b) << 16) | (uint32_t(c.g) << 8) | uint32_t(c.r);
}

void SpriteRenderer::SubmitSprite(uint16_t viewId, BgfxTexture* tex,
                                   const DrawRect<float>& src, const DrawRect<float>& dst,
                                   FlipMode flip, Color color)
{
  if (!tex || !bgfx::isValid(tex->Handle()))
    return;

  bgfx::TransientVertexBuffer tvb;
  if (!bgfx::getAvailTransientVertexBuffer(6, SpriteVertex::layout))
    return;
  bgfx::allocTransientVertexBuffer(&tvb, 6, SpriteVertex::layout);

  SpriteVertex* verts = reinterpret_cast<SpriteVertex*>(tvb.data);

  float texW = static_cast<float>(tex->w());
  float texH = static_cast<float>(tex->h());

  // Compute UVs from source rect
  float u0 = src.x / texW;
  float v0 = src.y / texH;
  float u1 = (src.x + src.w) / texW;
  float v1 = (src.y + src.h) / texH;

  // Apply flip
  if (static_cast<uint8_t>(flip) & static_cast<uint8_t>(FlipMode::Horizontal))
    std::swap(u0, u1);
  if (static_cast<uint8_t>(flip) & static_cast<uint8_t>(FlipMode::Vertical))
    std::swap(v0, v1);

  uint32_t abgr = PackColor(color);

  float x0 = dst.x;
  float y0 = dst.y;
  float x1 = dst.x + dst.w;
  float y1 = dst.y + dst.h;

  // Two triangles forming a quad
  verts[0] = { x0, y0, 0.0f, u0, v0, abgr };
  verts[1] = { x1, y0, 0.0f, u1, v0, abgr };
  verts[2] = { x1, y1, 0.0f, u1, v1, abgr };
  verts[3] = { x0, y0, 0.0f, u0, v0, abgr };
  verts[4] = { x1, y1, 0.0f, u1, v1, abgr };
  verts[5] = { x0, y1, 0.0f, u0, v1, abgr };

  bgfx::setVertexBuffer(0, &tvb);
  bgfx::setTexture(0, _texUniform, tex->Handle());

  uint64_t state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A
                 | BGFX_STATE_BLEND_ALPHA
                 | BGFX_STATE_MSAA;
  bgfx::setState(state);
  bgfx::submit(viewId, _spriteProgram);
}

void SpriteRenderer::SubmitRect(uint16_t viewId, const DrawRect<float>& dst, Color color, bool filled)
{
  if (filled)
  {
    bgfx::TransientVertexBuffer tvb;
    if (!bgfx::getAvailTransientVertexBuffer(6, SpriteVertex::layout))
      return;
    bgfx::allocTransientVertexBuffer(&tvb, 6, SpriteVertex::layout);

    SpriteVertex* verts = reinterpret_cast<SpriteVertex*>(tvb.data);
    uint32_t abgr = PackColor(color);

    float x0 = dst.x;
    float y0 = dst.y;
    float x1 = dst.x + dst.w;
    float y1 = dst.y + dst.h;

    verts[0] = { x0, y0, 0.0f, 0.0f, 0.0f, abgr };
    verts[1] = { x1, y0, 0.0f, 1.0f, 0.0f, abgr };
    verts[2] = { x1, y1, 0.0f, 1.0f, 1.0f, abgr };
    verts[3] = { x0, y0, 0.0f, 0.0f, 0.0f, abgr };
    verts[4] = { x1, y1, 0.0f, 1.0f, 1.0f, abgr };
    verts[5] = { x0, y1, 0.0f, 0.0f, 1.0f, abgr };

    bgfx::setVertexBuffer(0, &tvb);

    uint64_t state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A
                   | BGFX_STATE_BLEND_ALPHA
                   | BGFX_STATE_MSAA;
    bgfx::setState(state);
    bgfx::submit(viewId, _colorProgram);
  }
  else
  {
    // Outline: 4 line segments via PT_LINES (8 vertices, 4 lines)
    bgfx::TransientVertexBuffer tvb;
    if (!bgfx::getAvailTransientVertexBuffer(8, SpriteVertex::layout))
      return;
    bgfx::allocTransientVertexBuffer(&tvb, 8, SpriteVertex::layout);

    SpriteVertex* verts = reinterpret_cast<SpriteVertex*>(tvb.data);
    uint32_t abgr = PackColor(color);

    float x0 = dst.x;
    float y0 = dst.y;
    float x1 = dst.x + dst.w;
    float y1 = dst.y + dst.h;

    // Top
    verts[0] = { x0, y0, 0.0f, 0.0f, 0.0f, abgr };
    verts[1] = { x1, y0, 0.0f, 0.0f, 0.0f, abgr };
    // Right
    verts[2] = { x1, y0, 0.0f, 0.0f, 0.0f, abgr };
    verts[3] = { x1, y1, 0.0f, 0.0f, 0.0f, abgr };
    // Bottom
    verts[4] = { x1, y1, 0.0f, 0.0f, 0.0f, abgr };
    verts[5] = { x0, y1, 0.0f, 0.0f, 0.0f, abgr };
    // Left
    verts[6] = { x0, y1, 0.0f, 0.0f, 0.0f, abgr };
    verts[7] = { x0, y0, 0.0f, 0.0f, 0.0f, abgr };

    bgfx::setVertexBuffer(0, &tvb);

    uint64_t state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A
                   | BGFX_STATE_BLEND_ALPHA
                   | BGFX_STATE_PT_LINES;
    bgfx::setState(state);
    bgfx::submit(viewId, _colorProgram);
  }
}

void SpriteRenderer::SubmitLines(uint16_t viewId, const float* points, int nPoints, Color color)
{
  if (nPoints < 2)
    return;

  int numLineVerts = (nPoints - 1) * 2;
  bgfx::TransientVertexBuffer tvb;
  if (!bgfx::getAvailTransientVertexBuffer(numLineVerts, SpriteVertex::layout))
    return;
  bgfx::allocTransientVertexBuffer(&tvb, numLineVerts, SpriteVertex::layout);

  SpriteVertex* verts = reinterpret_cast<SpriteVertex*>(tvb.data);
  uint32_t abgr = PackColor(color);

  for (int i = 0; i < nPoints - 1; i++)
  {
    verts[i * 2 + 0] = { points[i * 2], points[i * 2 + 1], 0.0f, 0.0f, 0.0f, abgr };
    verts[i * 2 + 1] = { points[(i + 1) * 2], points[(i + 1) * 2 + 1], 0.0f, 0.0f, 0.0f, abgr };
  }

  bgfx::setVertexBuffer(0, &tvb);

  uint64_t state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A
                 | BGFX_STATE_BLEND_ALPHA
                 | BGFX_STATE_PT_LINES;
  bgfx::setState(state);
  bgfx::submit(viewId, _colorProgram);
}
