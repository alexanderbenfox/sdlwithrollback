#pragma once
#include "Rendering/GLTexture.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#if defined(_WIN32)
#include <gl/glu.h>
#else
#include <OpenGL/glu.h>
#endif

#include "Core/Math/Matrix4.h"
#include "Core/Geometry2D/Rect.h"

struct RenderContext
{
  bool glewInit = false;
  SDL_BlendMode blendMode = SDL_BlendMode::SDL_BLENDMODE_NONE;
};

struct RenderTextureCommand
{
  GLTexture* texture;
  DrawRect<float> srcRect;
  DrawRect<float> dstRect;
};

static RenderContext renderContext;

class OpenGLRenderer
{
private:
  static void SetBlendMode(RenderContext& context, SDL_BlendMode blendMode);
public:
  //! Renders to the current openGL context
  static void RenderQuad2D(const DrawRect<float>& dstRect, const double angle, const Vector2<float>* center, const SDL_Color color);
  static void RenderQuad2D(GLTexture* texture, const DrawRect<float>& srcRect, const DrawRect<float>& dstRect, const double angle, const Vector2<float>* center, const SDL_RendererFlip flip, const SDL_Color color);
  static void RenderLines2D(const SDL_Point* points, const int nPoints, const SDL_Color color);

  //! draws quad facing up centered at position
  static void RenderQuad3D(const SDL_Color color, const Vector2<float>& size, const Vector3<float>& position, const Vector3<float>& scale);
  static void RenderQuad3D(const RenderTextureCommand& cmd, const SDL_Color color, const Vector2<float>& size, const Vector3<float>& position, const Vector3<float>& scale);
  static void RenderCube3D(const SDL_Color* faceColors, const Vector3<float>& position, const Vector3<float>& scale);
  static void RenderPyramid3D(const SDL_Color* faceColors, const Vector3<float>& position, const Vector3<float>& scale);

};
