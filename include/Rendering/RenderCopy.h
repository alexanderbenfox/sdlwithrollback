#pragma once
#include "Rendering/GLTexture.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <gl/glu.h>


static const char* shaders[2] =
{
  /* vertex shader */
  "varying vec4 v_color;\n"
    "varying vec2 v_texCoord;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
    "    v_color = gl_Color;\n"
    "    v_texCoord = vec2(gl_MultiTexCoord0);\n"
    "}",
  /* fragment shader */
  "varying vec4 v_color;\n"
  "varying vec2 v_texCoord;\n"
  "uniform sampler2D tex0;\n"
  "\n"
  "void main()\n"
  "{\n"
  "    gl_FragColor = texture2D(tex0, v_texCoord) * v_color;\n"
  "}"
};

static const GLchar* vertexShaderSource[] =
{
    "#version 140\nin vec2 v_texCoord; void main() { gl_Position = vec4( v_texCoord.x, v_texCoord.y, 0, 1 );  }"
};

static const GLchar* fragmentShaderSource[] =
{
    "#version 140\nout vec4 LFragment; void main() { LFragment = vec4( 1.0, 1.0, 1.0, 1.0 ); }"
};

class GraphicsProgram
{
public:
  GraphicsProgram()
  {
    GLint GlewInitResult = glewInit();
    if (GLEW_OK != GlewInitResult)
    {
      printf("Unable to init glew: %s!\n", glewGetErrorString(GlewInitResult));
    }

    //Generate program
    gProgramID = glCreateProgram();
    //Create vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

    //Set vertex source
    glShaderSource(vertexShader, 1, vertexShaderSource, NULL);

    //Compile vertex source
    glCompileShader(vertexShader);

    //Check vertex shader for errors
    GLint vShaderCompiled = GL_FALSE;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vShaderCompiled);
    if (vShaderCompiled != GL_TRUE)
    {
      printf("Unable to compile vertex shader %d!\n", vertexShader);
      printShaderLog(vertexShader);
    }
    else
    {
      //Attach vertex shader to program
      glAttachShader(gProgramID, vertexShader);


      //Create fragment shader
      GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

      //Set fragment source
      glShaderSource(fragmentShader, 1, fragmentShaderSource, NULL);

      //Compile fragment source
      glCompileShader(fragmentShader);

      //Check fragment shader for errors
      GLint fShaderCompiled = GL_FALSE;
      glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fShaderCompiled);
      if (fShaderCompiled != GL_TRUE)
      {
        printf("Unable to compile fragment shader %d!\n", fragmentShader);
        printShaderLog(fragmentShader);
      }
      else
      {
        //Attach fragment shader to program
        glAttachShader(gProgramID, fragmentShader);


        //Link program
        glLinkProgram(gProgramID);

        //Check for errors
        GLint programSuccess = GL_TRUE;
        glGetProgramiv(gProgramID, GL_LINK_STATUS, &programSuccess);
        if (programSuccess != GL_TRUE)
        {
          printf("Error linking program %d!\n", gProgramID);
        }
        else
        {
          //Get vertex attribute location
          gVertexPos2DLocation = glGetAttribLocation(gProgramID, "v_texCoord");
          if (gVertexPos2DLocation == -1)
          {
            printf("v_texCoord is not a valid glsl program variable!\n");
          }
          else
          {
            //Initialize clear color
            glClearColor(0.f, 0.f, 0.f, 1.f);

            //VBO data
            GLfloat vertexData[] =
            {
                -0.5f, -0.5f,
                 0.5f, -0.5f,
                 0.5f,  0.5f,
                -0.5f,  0.5f
            };

            //IBO data
            GLuint indexData[] = { 0, 1, 2, 3 };

            //Create VBO
            glGenBuffers(1, &gVBO);
            glBindBuffer(GL_ARRAY_BUFFER, gVBO);
            glBufferData(GL_ARRAY_BUFFER, 2 * 4 * sizeof(GLfloat), vertexData, GL_STATIC_DRAW);

            //Create IBO
            glGenBuffers(1, &gIBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(GLuint), indexData, GL_STATIC_DRAW);
          }
        }
      }
    }
  }

  void printShaderLog(GLuint shader)
  {
    //Make sure name is shader
    if (glIsShader(shader))
    {
      //Shader log length
      int infoLogLength = 0;
      int maxLength = infoLogLength;

      //Get info string length
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

      //Allocate string
      char* infoLog = new char[maxLength];

      //Get info log
      glGetShaderInfoLog(shader, maxLength, &infoLogLength, infoLog);
      if (infoLogLength > 0)
      {
        //Print Log
        printf("%s\n", infoLog);
      }

      //Deallocate string
      delete[] infoLog;
    }
    else
    {
      printf("Name %d is not a shader\n", shader);
    }
  }

  void RenderPresent()
  {
    //Clear color buffer
    glClear(GL_COLOR_BUFFER_BIT);
    //Bind program
    glUseProgram(gProgramID);

    //Enable vertex position
    glEnableVertexAttribArray(gVertexPos2DLocation);

    //Set vertex data
    glBindBuffer(GL_ARRAY_BUFFER, gVBO);
    glVertexAttribPointer(gVertexPos2DLocation, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), NULL);
  }

  void EndPresent()
  {
    //Disable vertex position
    glDisableVertexAttribArray(gVertexPos2DLocation);

    //Unbind program
    glUseProgram(NULL);
  }

  GLuint gProgramID = 0;
  GLint gVertexPos2DLocation = -1;
  GLuint gVBO = 0;
  GLuint gIBO = 0;
};

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

static GraphicsProgram* program = nullptr;
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

  minu = (GLfloat)srcrect->x / texture->w;
  //minu *= texture->w;
  maxu = (GLfloat)(srcrect->x + srcrect->w) / texture->w;
  //maxu *= texture->w;
  minv = (GLfloat)srcrect->y / texture->h;
  //minv *= texture->h;
  maxv = (GLfloat)(srcrect->y + srcrect->h) / texture->h;
  //maxv *= texture->h;

  GL_SetBlendMode(renderContext, texture->blendMode);

  glBindTexture(GL_TEXTURE_2D, texture->id);
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