#pragma once
#include "Rendering/GLTexture.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <gl/glu.h>

struct RenderContext
{
  bool glewInit = false;
  SDL_BlendMode blendMode = SDL_BlendMode::SDL_BLENDMODE_NONE;
};

static void GL_SetBlendMode(RenderContext& context, SDL_BlendMode blendMode)
{
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
    }
    context.blendMode = blendMode;
  }
}

static RenderContext renderContext;

//! Renders to the current openGL context
static int GL_RenderCopyEx(GLTexture* texture, const SDL_Rect* srcrect, const SDL_Rect* dstrect, const double angle, const SDL_Point* center, const SDL_RendererFlip flip)
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

  glTranslatef(-((GLfloat)dstrect->x + centerx), -((GLfloat)dstrect->y + centery), (GLfloat)0.0);
  glPopMatrix();

  glDisable(GL_TEXTURE_2D);
  glDisableClientState(GL_VERTEX_ARRAY);

  return 0;
}