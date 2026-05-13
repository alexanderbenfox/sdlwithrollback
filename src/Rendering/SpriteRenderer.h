#pragma once
#include <bgfx/bgfx.h>
#include "Rendering/RenderTypes.h"
#include "Core/Geometry2D/Rect.h"

class BgfxTexture;

struct SpriteVertex
{
  float x, y, z;
  float u, v;
  uint32_t abgr;

  static bgfx::VertexLayout layout;
  static void Init();
};

class SpriteRenderer
{
public:
  void Init();
  void Shutdown();

  void SubmitSprite(uint16_t viewId, BgfxTexture* tex,
                    const DrawRect<float>& src, const DrawRect<float>& dst,
                    FlipMode flip, Color color);

  void SubmitRect(uint16_t viewId, const DrawRect<float>& dst, Color color, bool filled);

  void SubmitLines(uint16_t viewId, const float* points, int nPoints, Color color);

private:
  bgfx::ProgramHandle _spriteProgram;
  bgfx::ProgramHandle _colorProgram;
  bgfx::UniformHandle _texUniform;
};
