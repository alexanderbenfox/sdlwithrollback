#include "Rendering/OpenGLRenderer.h"

//______________________________________________________________________________
void OpenGLRenderer::SetBlendMode(RenderContext& context, SDL_BlendMode blendMode)
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

//______________________________________________________________________________
void OpenGLRenderer::RenderQuad2D(const DrawRect<float>& dstRect, const double angle, const Vector2<float>* center, const SDL_Color color)
{
  GLfloat centerx, centery;

  if (center)
  {
    centerx = center->x;
    centery = center->y;
  }
  else
  {
    centerx = dstRect.w / 2.0f;
    centery = dstRect.h / 2.0f;
  }

  glPushMatrix();

  glTranslatef((GLfloat)dstRect.x + centerx, (GLfloat)dstRect.y + centery, (GLfloat)0.0);
  glRotated(angle, (GLdouble)0.0, (GLdouble)0.0, (GLdouble)1.0);

  glColor4ub((GLubyte)color.r, (GLubyte)color.g, (GLubyte)color.b, (GLubyte)color.a);

  glBegin(GL_QUADS);
  glVertex2f(-dstRect.w / 2.0f, -dstRect.h / 2.0f);
  glVertex2f(dstRect.w / 2.0f, -dstRect.h / 2.0f);
  glVertex2f(dstRect.w / 2.0f, dstRect.h / 2.0f);
  glVertex2f(-dstRect.w / 2.0f, dstRect.h / 2.0f);
  glEnd();

  //reset color
  glColor4ub((GLubyte)255, (GLubyte)255, (GLubyte)255, (GLubyte)255);

  glTranslatef(-((GLfloat)dstRect.x + centerx), -((GLfloat)dstRect.y + centery), (GLfloat)0.0);
  glPopMatrix();
}

//______________________________________________________________________________
void OpenGLRenderer::RenderQuad2D(GLTexture* texture, const DrawRect<float>& srcRect, const DrawRect<float>& dstRect, const double angle, const Vector2<float>* center, const SDL_RendererFlip flip, const SDL_Color color)
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
    centerx = dstRect.w / 2.0f;
    centery = dstRect.h / 2.0f;
  }

  if (flip & SDL_FLIP_HORIZONTAL) {
    minx = dstRect.w - centerx;
    maxx = -centerx;
  }
  else {
    minx = -centerx;
    maxx = dstRect.w - centerx;
  }

  if (flip & SDL_FLIP_VERTICAL) {
    miny = dstRect.h - centery;
    maxy = -centery;
  }
  else {
    miny = -centery;
    maxy = dstRect.h - centery;
  }

  minu = (GLfloat)srcRect.x / texture->w();
  maxu = (GLfloat)(srcRect.x + srcRect.w) / texture->w();
  minv = (GLfloat)srcRect.y / texture->h();
  maxv = (GLfloat)(srcRect.y + srcRect.h) / texture->h();

  SetBlendMode(renderContext, texture->BlendMode());

  glBindTexture(GL_TEXTURE_2D, texture->ID());
  glEnable(GL_TEXTURE_2D);
  // Translate to flip, rotate, translate to position
  glPushMatrix();
  glTranslatef((GLfloat)dstRect.x + centerx, (GLfloat)dstRect.y + centery, (GLfloat)0.0);
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

  glTranslatef(-((GLfloat)dstRect.x + centerx), -((GLfloat)dstRect.y + centery), (GLfloat)0.0);
  glPopMatrix();

  glDisable(GL_TEXTURE_2D);
  glDisableClientState(GL_VERTEX_ARRAY);
}

//______________________________________________________________________________
void OpenGLRenderer::RenderLines2D(const Vector2<float>* points, const int nPoints, const SDL_Color color)
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

//______________________________________________________________________________
void OpenGLRenderer::RenderQuad3D(const SDL_Color color, const Vector2<float>& size, const Vector3<float>& position, const Vector3<float>& scale)
{
  glTranslatef(position.x, position.y, position.z);

  auto scaler = Mat4::Scale(scale.x, scale.y, scale.z);
  float m[16];
  Mat4::toMat4(scaler, m);
  glMultMatrixf(m);

  // bottom
  glBegin(GL_QUADS);
  glColor4ub((GLubyte)color.r, (GLubyte)color.g, (GLubyte)color.b, (GLubyte)color.a);
  glVertex3f(size.x / 2.0f, 0.0f, -size.y / 2.0f);
  glVertex3f(-size.x / 2.0f, 0.0f, -size.y / 2.0f);
  glVertex3f(-size.x / 2.0f, 0.0f, size.y / 2.0f);
  glVertex3f(size.x / 2.0f, 0.0f, size.y / 2.0f);
  glEnd();
}

//______________________________________________________________________________
void OpenGLRenderer::RenderQuad3D(const RenderTextureCommand& cmd, const SDL_Color color, const Vector2<float>& size, const Vector3<float>& position, const Vector3<float>& scale)
{
  GLfloat minu, maxu, minv, maxv;
  minu = (GLfloat)cmd.srcRect.x / cmd.texture->w();
  maxu = (GLfloat)(cmd.srcRect.x + cmd.srcRect.w) / cmd.texture->w();
  minv = (GLfloat)cmd.srcRect.y / cmd.texture->h();
  maxv = (GLfloat)(cmd.srcRect.y + cmd.srcRect.h) / cmd.texture->h();

  glPushMatrix();
  glTranslatef(position.x, position.y, position.z);

  SetBlendMode(renderContext, cmd.texture->BlendMode());
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, cmd.texture->ID());

  auto scaler = Mat4::Scale(scale.x, scale.y, scale.z);
  float m[16];
  Mat4::toMat4(scaler, m);
  glMultMatrixf(m);

  glColor4ub((GLubyte)color.r, (GLubyte)color.g, (GLubyte)color.b, (GLubyte)color.a);

  // bottom
  glBegin(GL_QUADS);
  glTexCoord2f(maxu, minv);
  glVertex3f(size.x / 2.0f, 0.0f, -size.y / 2.0f);
  glTexCoord2f(minu, minv);
  glVertex3f(-size.x / 2.0f, 0.0f, -size.y / 2.0f);
  glTexCoord2f(minu, maxv);
  glVertex3f(-size.x / 2.0f, 0.0f, size.y / 2.0f);
  glTexCoord2f(maxu, maxv);
  glVertex3f(size.x / 2.0f, 0.0f, size.y / 2.0f);
  glEnd();

  glDisable(GL_TEXTURE_2D);

  glPopMatrix();
}


//______________________________________________________________________________
void OpenGLRenderer::RenderCube3D(const SDL_Color* faceColors, const Vector3<float>& position, const Vector3<float>& scale)
{
  glTranslatef(position.x, position.y, position.z);

  auto scaler = Mat4::Scale(scale.x, scale.y, scale.z);
  float m[16];
  Mat4::toMat4(scaler, m);
  glMultMatrixf(m);


  glBegin(GL_QUADS);                // Begin drawing the color cube with 6 quads
     // Top face (y = 1.0f)
     // Define vertices in counter-clockwise (CCW) order with normal pointing out
  glColor4ub((GLubyte)faceColors[0].r, (GLubyte)faceColors[0].g, (GLubyte)faceColors[0].b, (GLubyte)faceColors[0].a);
  glVertex3f(1.0f, 1.0f, -1.0f);
  glVertex3f(-1.0f, 1.0f, -1.0f);
  glVertex3f(-1.0f, 1.0f, 1.0f);
  glVertex3f(1.0f, 1.0f, 1.0f);

  // Bottom face (y = -1.0f)
  glColor4ub((GLubyte)faceColors[1].r, (GLubyte)faceColors[1].g, (GLubyte)faceColors[1].b, (GLubyte)faceColors[1].a);
  glVertex3f(1.0f, -1.0f, 1.0f);
  glVertex3f(-1.0f, -1.0f, 1.0f);
  glVertex3f(-1.0f, -1.0f, -1.0f);
  glVertex3f(1.0f, -1.0f, -1.0f);

  // Front face  (z = 1.0f)
  glColor4ub((GLubyte)faceColors[2].r, (GLubyte)faceColors[2].g, (GLubyte)faceColors[2].b, (GLubyte)faceColors[2].a);
  glVertex3f(1.0f, 1.0f, 1.0f);
  glVertex3f(-1.0f, 1.0f, 1.0f);
  glVertex3f(-1.0f, -1.0f, 1.0f);
  glVertex3f(1.0f, -1.0f, 1.0f);

  // Back face (z = -1.0f)
  glColor4ub((GLubyte)faceColors[3].r, (GLubyte)faceColors[3].g, (GLubyte)faceColors[3].b, (GLubyte)faceColors[3].a);
  glVertex3f(1.0f, -1.0f, -1.0f);
  glVertex3f(-1.0f, -1.0f, -1.0f);
  glVertex3f(-1.0f, 1.0f, -1.0f);
  glVertex3f(1.0f, 1.0f, -1.0f);

  // Left face (x = -1.0f)
  glColor4ub((GLubyte)faceColors[4].r, (GLubyte)faceColors[4].g, (GLubyte)faceColors[4].b, (GLubyte)faceColors[4].a);
  glVertex3f(-1.0f, 1.0f, 1.0f);
  glVertex3f(-1.0f, 1.0f, -1.0f);
  glVertex3f(-1.0f, -1.0f, -1.0f);
  glVertex3f(-1.0f, -1.0f, 1.0f);

  // Right face (x = 1.0f)
  glColor4ub((GLubyte)faceColors[5].r, (GLubyte)faceColors[5].g, (GLubyte)faceColors[5].b, (GLubyte)faceColors[5].a);
  glVertex3f(1.0f, 1.0f, -1.0f);
  glVertex3f(1.0f, 1.0f, 1.0f);
  glVertex3f(1.0f, -1.0f, 1.0f);
  glVertex3f(1.0f, -1.0f, -1.0f);
  glEnd();  // End of drawing color-cube

}

//______________________________________________________________________________
void OpenGLRenderer::RenderPyramid3D(const SDL_Color* faceColors, const Vector3<float>& position, const Vector3<float>& scale)
{
  glTranslatef(position.x, position.y, position.z);

  auto scaler = Mat4::Scale(scale.x, scale.y, scale.z);
  float m[16];
  Mat4::toMat4(scaler, m);
  glMultMatrixf(m);

  glBegin(GL_TRIANGLES);           // Begin drawing the pyramid with 4 triangles
     // Front
  glColor4ub((GLubyte)faceColors[0].r, (GLubyte)faceColors[0].g, (GLubyte)faceColors[0].b, (GLubyte)faceColors[0].a);
  glVertex3f(0.0f, 1.0f, 0.0f);
  glColor4ub((GLubyte)faceColors[1].r, (GLubyte)faceColors[1].g, (GLubyte)faceColors[1].b, (GLubyte)faceColors[1].a);
  glVertex3f(-1.0f, -1.0f, 1.0f);
  glColor4ub((GLubyte)faceColors[2].r, (GLubyte)faceColors[2].g, (GLubyte)faceColors[2].b, (GLubyte)faceColors[2].a);
  glVertex3f(1.0f, -1.0f, 1.0f);

  // Right
  glColor4ub((GLubyte)faceColors[0].r, (GLubyte)faceColors[0].g, (GLubyte)faceColors[0].b, (GLubyte)faceColors[0].a);
  glVertex3f(0.0f, 1.0f, 0.0f);
  glColor4ub((GLubyte)faceColors[2].r, (GLubyte)faceColors[2].g, (GLubyte)faceColors[2].b, (GLubyte)faceColors[2].a);
  glVertex3f(1.0f, -1.0f, 1.0f);
  glColor4ub((GLubyte)faceColors[1].r, (GLubyte)faceColors[1].g, (GLubyte)faceColors[1].b, (GLubyte)faceColors[1].a);
  glVertex3f(1.0f, -1.0f, -1.0f);

  // Back
  glColor4ub((GLubyte)faceColors[0].r, (GLubyte)faceColors[0].g, (GLubyte)faceColors[0].b, (GLubyte)faceColors[0].a);
  glVertex3f(0.0f, 1.0f, 0.0f);
  glColor4ub((GLubyte)faceColors[1].r, (GLubyte)faceColors[1].g, (GLubyte)faceColors[1].b, (GLubyte)faceColors[1].a);
  glVertex3f(1.0f, -1.0f, -1.0f);
  glColor4ub((GLubyte)faceColors[2].r, (GLubyte)faceColors[2].g, (GLubyte)faceColors[2].b, (GLubyte)faceColors[2].a);
  glVertex3f(-1.0f, -1.0f, -1.0f);

  // Left
  glColor4ub((GLubyte)faceColors[0].r, (GLubyte)faceColors[0].g, (GLubyte)faceColors[0].b, (GLubyte)faceColors[0].a);
  glVertex3f(0.0f, 1.0f, 0.0f);
  glColor4ub((GLubyte)faceColors[2].r, (GLubyte)faceColors[2].g, (GLubyte)faceColors[2].b, (GLubyte)faceColors[2].a);
  glVertex3f(-1.0f, -1.0f, -1.0f);
  glColor4ub((GLubyte)faceColors[1].r, (GLubyte)faceColors[1].g, (GLubyte)faceColors[1].b, (GLubyte)faceColors[1].a);
  glVertex3f(-1.0f, -1.0f, 1.0f);
  glEnd();   // Done drawing the pyramid
}
