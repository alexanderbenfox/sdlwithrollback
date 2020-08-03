#include "Rendering/RenderManager.h"
#include "GameManagement.h"
#include "Rendering/OpenGLRenderer.h"

#include "Components/Camera.h"

#include <type_traits>

#if defined(_WIN32)
#include <gl/glut.h>
#else
#include <GLUT/glut.h>
#endif

//! Title of the game in the window
const char* Title = "Duel Engine";

//______________________________________________________________________________
template <> void DrawOperator<BlitOperation<GLTexture>>::DoDraw(BlitOperation<GLTexture>& operation)
{
  if (!operation.valid) return;
  auto srcTexture = operation.textureResource->Get();
  float rotation = 0;

  try
  {
    OpenGLRenderer::RenderQuad2D(srcTexture, operation.srcRect, operation.targetRect, rotation, nullptr, operation.flip, operation.displayColor);
  }
  catch (std::exception& e)
  {
    std::cout << "I guess this texture isn't valid??" << "\nCaught exception: " << e.what() << "\n";
  }
}

//______________________________________________________________________________
template <> void DrawOperator<DrawPrimitive<GLTexture>>::DoDraw(DrawPrimitive<GLTexture>& operation)
{
  if (!operation.valid) return;

  if (operation.filled)
    OpenGLRenderer::RenderQuad2D(operation.targetRect, 0, nullptr, operation.displayColor);
  else
  {
    int xBeg = operation.targetRect.x;
    int yBeg = operation.targetRect.y;
    int xEnd = operation.targetRect.x + operation.targetRect.w;
    int yEnd = operation.targetRect.y + operation.targetRect.h;

    SDL_Point points[5] =
    {
      {xBeg, yBeg},
      {xBeg, yEnd},
      {xEnd, yEnd},
      {xEnd, yBeg},
      {xBeg, yBeg}
    };
    OpenGLRenderer::RenderLines2D(points, 5, operation.displayColor);
  }
}

/*template <> void DrawOperator<BlitOperation<GLTexture>>::SetupCamera()
{
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  const Matrix4F& matrix = GameManager::Get().GetMainCamera()->matrix;
  float m[16];
  Mat4::toMat4(matrix, m);
  glMultMatrixf(m);
}

template <> void DrawOperator<BlitOperation<GLTexture>>::UndoCamera()
{
  const Matrix4F& matrix = GameManager::Get().GetMainCamera()->matrix;
  const Matrix4F invTranspose = matrix.Transpose() * -1.0f;
  float m[16];
  Mat4::toMat4(invTranspose, m);

  // unset the camera matrix
  glMultMatrixf(m);
  // pop matrix
  glPopMatrix();
}*/

template <typename Drawable>
void DrawOperator<Drawable>::SetupCamera(Camera* camera)
{
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, m_nativeWidth, m_nativeHeight, 0, 0, 16);

  const Matrix4F& matrix = camera->matrix;
  Vector3<float> position = Mat4::GetPosition(matrix);
  glTranslatef(position.x, position.y, position.z);
}

template <typename Drawable>
void DrawOperator<Drawable>::UndoCamera(Camera* camera)
{
  const Matrix4F& matrix = camera->matrix;
  Vector3<float> negativePos = -Mat4::GetPosition(matrix);

  // unset the camera matrix
  glTranslatef(negativePos.x, negativePos.y, negativePos.z);
  // pop matrix
  glPopMatrix();
}


//______________________________________________________________________________
RenderManager::RenderManager() :
  _renderer(nullptr),
  _window(nullptr),
  _glContext(nullptr),
  _renderScale(1.0, 1.0) {}

//______________________________________________________________________________
void RenderManager::Init()
{
  SDL_Init(SDL_INIT_EVERYTHING | SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK);
  TTF_Init();

  _window = SDL_CreateWindow(Title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    m_nativeWidth, m_nativeHeight,
    SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);

  _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);
  SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND);

  // init the gl context and a bunch of other stuff to enable GL
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

  _glContext = SDL_GL_CreateContext(_window);
  SDL_GL_MakeCurrent(_window, _glContext);

  // Enable vsync
  SDL_GL_SetSwapInterval(1);
  SDL_RenderSetScale(_renderer, 1.0f, 1.0f);

#ifndef _WIN32
  _sdlWindowFormat = SDL_GetWindowPixelFormat(_window);
#else
  _sdlWindowFormat = SDL_PIXELFORMAT_RGBA8888;
#endif
}

//______________________________________________________________________________
void RenderManager::Destroy()
{
  SDL_DestroyRenderer(_renderer);
  SDL_DestroyWindow(_window);
  SDL_GL_DeleteContext(_glContext);

  _renderer = nullptr;
  _window = nullptr;

  SDL_Quit();
  TTF_Quit();

}

//______________________________________________________________________________
void RenderManager::ProcessResizeEvent(const SDL_Event& event)
{
  // get the window event size
  Vector2<int> newWindowSize(event.window.data1, event.window.data2);

  // get new scale for renderer
  _renderScale = Vector2<double>(
    static_cast<double>(newWindowSize.x) / static_cast<double>(m_nativeWidth),
    static_cast<double>(newWindowSize.y) / static_cast<double>(m_nativeHeight));

  // set render scale
  SDL_RenderSetScale(_renderer, static_cast<float>(_renderScale.x), static_cast<float>(_renderScale.y));
}

//______________________________________________________________________________
void RenderManager::SwitchTo2D()
{
  glEnable(GL_TEXTURE_2D);
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glMatrixMode(GL_MODELVIEW);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, m_nativeWidth, m_nativeHeight, 0, 0, 16);
  glMatrixMode(GL_MODELVIEW);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
}

//______________________________________________________________________________
void RenderManager::SwitchTo3D()
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(54.0f, (float)m_nativeWidth / m_nativeHeight, 1.0f, 1000);
  glMatrixMode(GL_MODELVIEW);

  glDisable(GL_BLEND);
  glCullFace(GL_BACK);
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
}

//______________________________________________________________________________
void RenderManager::Draw()
{
  SwitchTo3D();
  Draw3DBackground();
  SwitchTo2D();
  for (int layer = 0; layer < (int)RenderLayer::NLayers; layer++)
  {
    if (_drawers[layer].camera)
    {
      _drawers[layer].textureDrawer.PerformDraw(_drawers[layer].camera);
      _drawers[layer].primitiveDrawer.PerformDraw(_drawers[layer].camera);
    }
  }
}

//______________________________________________________________________________
void RenderManager::Clear()
{
  // clear previous render
  glClearColor(0.0, 0.0, 0.0, 1);
  glClear(GL_COLOR_BUFFER_BIT);

  // change display color
  glColor4f(1.0, 1.0, 1.0, 1.0);
}

//______________________________________________________________________________
void RenderManager::Present()
{
  SDL_GL_SwapWindow(_window);
}

//______________________________________________________________________________
void RenderManager::Draw3DBackground()
{
  bool camera = _drawers[(int)RenderLayer::World].camera != nullptr;
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear color and depth buffers
  glMatrixMode(GL_MODELVIEW);     // To operate on model-view matrix
  glPushMatrix();
  Matrix4F matrix;
  if(camera)
    matrix = _drawers[(int)RenderLayer::World].camera->worldMatrix;

  auto SetUpMatrix = [this, camera, &matrix]()
  {
    if (camera)
    {
      glLoadIdentity();
      Vector3<float> camPos = Mat4::GetPosition(matrix);
      gluLookAt(camPos.x, camPos.y, 5.0, camPos.x, camPos.y, 0.0, 0.0, 1.0, 0.0);
    }
  };

  auto UnSetUpMatrix = [this, camera, &matrix]()
  {
    if (camera)
    {
      const Matrix4F invTranspose = matrix.Transpose() * -1.0f;
      float m[16];
      Mat4::toMat4(invTranspose, m);
    }
  };
  
  const Vector2<float> stageSize(6.0f, 4.0f);
  float m[16];

  SetUpMatrix();
  glTranslatef(0.0f, 0.0f, 3.0f);
  OpenGLRenderer::RenderQuad3D({ 230, 230, 230, 255 }, stageSize, Vector3<float>(0, -1.0f, 0.0f), Vector3<float>(1.0f, 1.0f, 1.0f));
  
  SetUpMatrix();
  glTranslatef(0.0f, 0.0f, 3.0f);
  Mat4::toMat4(Mat4::RotateZ180, m);
  glMultMatrixf(m);
  OpenGLRenderer::RenderQuad3D({ 153, 153, 153, 255 }, stageSize, Vector3<float>(0, -1.0f, 0.0f), Vector3<float>(1.0f, 1.0f, 1.0f));

  SetUpMatrix();
  glTranslatef(0.0f, 0.0f, 3.0f);
  Mat4::toMat4(Mat4::RotateZ90, m);
  glMultMatrixf(m);
  OpenGLRenderer::RenderQuad3D({ 128, 128, 128, 255 }, stageSize, Vector3<float>(0, -3.0f, 0.0f), Vector3<float>(1.0f, 1.0f, 1.0f));

  SetUpMatrix();
  glTranslatef(0.0f, 0.0f, 3.0f);
  Mat4::toMat4(Mat4::RotateZN90, m);
  glMultMatrixf(m);
  OpenGLRenderer::RenderQuad3D({ 128, 128, 128, 255 }, stageSize, Vector3<float>(0, -3.0f, 0.0f), Vector3<float>(1.0f, 1.0f, 1.0f));

  
  SetUpMatrix();
  const SDL_Color cubeColors[6] = { {0, 255, 0, 255}, {255, 128, 0, 255}, {255, 0, 0, 255}, {255, 255, 0, 255}, {0, 0, 255, 255}, {255, 0, 255, 255} };
  OpenGLRenderer::RenderCube3D(cubeColors, Vector3<float>(1.0f, 0.0f, 2.0f), Vector3<float>(0.4f, 0.4f, 0.4f));

  SetUpMatrix();
  const SDL_Color pyramidColors[3] = { {255, 0, 0, 255}, {0, 255, 0, 255}, {0, 0, 255, 255} };
  OpenGLRenderer::RenderPyramid3D(pyramidColors, Vector3<float>(-1.2f, 0.0f, 2.7f), Vector3<float>(0.3f, 0.3f, 0.3f));
  UnSetUpMatrix();

  glPopMatrix();
}
