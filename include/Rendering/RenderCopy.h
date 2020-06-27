#pragma once
#include "Rendering/GLTexture.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#if defined(_WIN32)
#include <gl/glu.h>
#else
#include <OpenGL/glu.h>
#endif

struct RenderContext
{
  bool glewInit = false;
  SDL_BlendMode blendMode = SDL_BlendMode::SDL_BLENDMODE_NONE;
};

static void GL_SetBlendMode(RenderContext& context, SDL_BlendMode blendMode)
{
#if defined(_WIN32)
  if (!context.glewInit)
  {
    GLint GlewInitResult = glewInit();
    if (GLEW_OK != GlewInitResult)
    {
      printf("Unable to init glew: %s!\n", glewGetErrorString(GlewInitResult));
    }
    else
    {
      context.glewInit = true;
    }
  }
#endif

  if (blendMode != context.blendMode) {
    switch (blendMode) {
    case SDL_BLENDMODE_NONE:
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
      glDisable(GL_BLEND);
      break;
    case SDL_BLENDMODE_BLEND:
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      glEnable(GL_BLEND);
      glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
      break;
    case SDL_BLENDMODE_ADD:
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE);
      break;
    case SDL_BLENDMODE_MOD:
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      glEnable(GL_BLEND);
      glBlendFunc(GL_ZERO, GL_SRC_COLOR);
      break;
    case SDL_BLENDMODE_INVALID:
      throw std::invalid_argument("Invalid blend mode");
      break;
    default:
      break;
    }
    context.blendMode = blendMode;
  }
}

static RenderContext renderContext;

//! Renders to the current openGL context
static int GL_RenderCopyEx(GLTexture* texture, const SDL_Rect* srcrect, const SDL_Rect* dstrect,
  const double angle, const SDL_Point* center, const SDL_RendererFlip flip, const SDL_Color color)
{
  GLfloat minx, miny, maxx, maxy;
  GLfloat centerx, centery;
  GLfloat minu, maxu, minv, maxv;

  //GL_SetShader(data, SHADER_RGB);

  if (center)
  {
    centerx = center->x;
    centery = center->y;
  }
  else
  {
    centerx = dstrect->w / 2;
    centery = dstrect->h / 2;
  }

  if (flip & SDL_FLIP_HORIZONTAL) {
    minx = dstrect->w - centerx;
    maxx = -centerx;
  }
  else {
    minx = -centerx;
    maxx = dstrect->w - centerx;
  }

  if (flip & SDL_FLIP_VERTICAL) {
    miny = dstrect->h - centery;
    maxy = -centery;
  }
  else {
    miny = -centery;
    maxy = dstrect->h - centery;
  }

  minu = (GLfloat)srcrect->x / texture->w();
  //minu *= texture->w;
  maxu = (GLfloat)(srcrect->x + srcrect->w) / texture->w();
  //maxu *= texture->w;
  minv = (GLfloat)srcrect->y / texture->h();
  //minv *= texture->h;
  maxv = (GLfloat)(srcrect->y + srcrect->h) / texture->h();
  //maxv *= texture->h;

  GL_SetBlendMode(renderContext, texture->BlendMode());

  glBindTexture(GL_TEXTURE_2D, texture->ID());
  glEnable(GL_TEXTURE_2D);
  // Translate to flip, rotate, translate to position
  glPushMatrix();
  glTranslatef((GLfloat)dstrect->x + centerx, (GLfloat)dstrect->y + centery, (GLfloat)0.0);
  glRotated(angle, (GLdouble)0.0, (GLdouble)0.0, (GLdouble)1.0);

  glColor4ub((GLubyte)color.r, (GLubyte)color.g, (GLubyte)color.b, (GLubyte)color.a);

  //program->RenderPresent();
  glBegin(GL_TRIANGLE_STRIP);
  glTexCoord2f(minu, minv);
  glVertex2f(minx, miny);
  glTexCoord2f(maxu, minv);
  glVertex2f(maxx, miny);
  glTexCoord2f(minu, maxv);
  glVertex2f(minx, maxy);
  glTexCoord2f(maxu, maxv);
  glVertex2f(maxx, maxy);
  glEnd();
  //program->EndPresent();

  glColor4ub((GLubyte)255, (GLubyte)255, (GLubyte)255, (GLubyte)255);

  glTranslatef(-((GLfloat)dstrect->x + centerx), -((GLfloat)dstrect->y + centery), (GLfloat)0.0);
  glPopMatrix();

  glDisable(GL_TEXTURE_2D);
  glDisableClientState(GL_VERTEX_ARRAY);

  return 0;
}

static void GL_RenderDrawLines(const SDL_Point* points, const int nPoints, const SDL_Color color)
{
  for (int i = 0; i < nPoints; ++i)
  {
    int p1Idx = i;
    int p2Idx = (i + 1) % nPoints;

    glColor4ub((GLubyte)color.r, (GLubyte)color.g, (GLubyte)color.b, (GLubyte)color.a);

    glBegin(GL_LINES);
    glVertex2f(points[p1Idx].x, points[p1Idx].y);
    glVertex2f(points[p2Idx].x, points[p2Idx].y);
    glEnd();

    //reset color
    glColor4ub((GLubyte)255, (GLubyte)255, (GLubyte)255, (GLubyte)255);
  }
}

static void GL_RenderDrawRectangle(Vector2<int> beg, Vector2<int> end, const SDL_Color color)
{
  glColor4ub((GLubyte)color.r, (GLubyte)color.g, (GLubyte)color.b, (GLubyte)color.a);

  glBegin(GL_QUADS);
  glVertex2f(beg.x, beg.y);
  glVertex2f(end.x, beg.y);
  glVertex2f(end.x, end.y);
  glVertex2f(beg.x, end.y);
  glEnd();

  //reset color
  glColor4ub((GLubyte)255, (GLubyte)255, (GLubyte)255, (GLubyte)255);
}