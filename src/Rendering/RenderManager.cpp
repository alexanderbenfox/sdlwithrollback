#include "Rendering/RenderManager.h"
#include "GameManagement.h"
#include "Rendering/RenderCopy.h"

#include "Components/Camera.h"

#include <type_traits>

//! Title of the game in the window... will figure this out lateerrrrr
const char* Title = "Game Title";

//______________________________________________________________________________
template <> void DrawOperator<BlitOperation<GLTexture>>::DoDraw(SDL_Renderer* renderer, BlitOperation<GLTexture>& operation)
{
  if (!operation.valid) return;

  //GameManager::Get().GetMainCamera()->ConvScreenSpace(&operation);
  //if (GameManager::Get().GetMainCamera()->EntityInDisplay(&operation))
  //{
    auto srcTexture = operation.textureResource->Get();
    float rotation = 0;

    try
    {
      GL_RenderCopyEx(srcTexture, &operation.textureRect, &operation.displayRect, rotation, nullptr, operation.flip, operation.displayColor);
    }
    catch (std::exception& e)
    {
      std::cout << "I guess this texture isn't valid??" << "\nCaught exception: " << e.what() << "\n";
    }
  //}
}

//______________________________________________________________________________
template <> void DrawOperator<BlitOperation<SDL_Texture>>::DoDraw(SDL_Renderer* renderer, BlitOperation<SDL_Texture>& operation)
{
  if (!operation.valid) return;

  //GameManager::Get().GetMainCamera()->ConvScreenSpace(&operation);
  //if (GameManager::Get().GetMainCamera()->EntityInDisplay(&operation))
  //{
    auto srcTexture = operation.textureResource->Get();
    float rotation = 0;

    try
    {
      int w, h;
      if (SDL_QueryTexture(operation.textureResource->Get(), NULL, NULL, &w, &h) == 0)
      {
        SDL_SetTextureColorMod(operation.textureResource->Get(), operation.displayColor.r, operation.displayColor.g, operation.displayColor.b);
        SDL_RenderCopyEx(renderer, srcTexture, &operation.textureRect, &operation.displayRect, rotation, nullptr, operation.flip);
      }
    }
    catch (std::exception& e)
    {
      std::cout << "I guess this texture isn't valid??" << "\nCaught exception: " << e.what() << "\n";
    }
  //}
}

//______________________________________________________________________________
template <> void DrawOperator<DrawPrimitive<GLTexture>>::DoDraw(SDL_Renderer* renderer, DrawPrimitive<GLTexture>& operation)
{
  if (!operation.valid) return;

  //GameManager::Get().GetMainCamera()->ConvScreenSpace(&operation);
  //if (GameManager::Get().GetMainCamera()->EntityInDisplay(&operation))
  {
    int xBeg = operation.displayRect.x;
    int yBeg = operation.displayRect.y;
    int xEnd = xBeg + operation.displayRect.w;
    int yEnd = yBeg + operation.displayRect.h;

    SDL_Point points[5] =
    {
      {xBeg, yBeg},
      {xBeg, yEnd},
      {xEnd, yEnd},
      {xEnd, yBeg},
      {xBeg, yBeg}
    };

    if (operation.filled)
      GL_RenderDrawRectangle(Vector2<int>(xBeg, yBeg), Vector2<int>(xEnd, yEnd), operation.displayColor);
    else
      GL_RenderDrawLines(points, 5, operation.displayColor);
  }
}

//______________________________________________________________________________
template <> void DrawOperator<DrawPrimitive<SDL_Texture>>::DoDraw(SDL_Renderer* renderer, DrawPrimitive<SDL_Texture>& operation)
{
  if (!operation.valid) return;

  //GameManager::Get().GetMainCamera()->ConvScreenSpace(&operation);
  //if (GameManager::Get().GetMainCamera()->EntityInDisplay(&operation))
  {
    if (!operation.filled)
    {
      int xBeg = operation.displayRect.x;
      int yBeg = operation.displayRect.y;
      int xEnd = xBeg + operation.displayRect.w;
      int yEnd = yBeg + operation.displayRect.h;

      SDL_Point points[5] =
      {
        {xBeg, yBeg},
        {xBeg, yEnd},
        {xEnd, yEnd},
        {xEnd, yBeg},
        {xBeg, yBeg}
      };

      SDL_SetRenderDrawColor(GRenderer.GetRenderer(), operation.displayColor.r, operation.displayColor.g, operation.displayColor.b, operation.displayColor.a);
      SDL_RenderDrawLines(GRenderer.GetRenderer(), points, 5);
      SDL_SetRenderDrawColor(GRenderer.GetRenderer(), 0, 0, 0, SDL_ALPHA_OPAQUE);
    }
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

template <> void DrawOperator<BlitOperation<GLTexture>>::SetupCamera(Camera* camera)
{
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, m_nativeWidth, m_nativeHeight, 0, 0, 16);

  const Matrix4F& matrix = camera->matrix;
  Vector3<float> position = Mat4::GetPosition(matrix);
  glTranslatef(position.x, position.y, position.z);
}

template <> void DrawOperator<BlitOperation<GLTexture>>::UndoCamera(Camera* camera)
{
  const Matrix4F& matrix = camera->matrix;
  Vector3<float> negativePos = -Mat4::GetPosition(matrix);

  // unset the camera matrix
  glTranslatef(negativePos.x, negativePos.y, negativePos.z);
  // pop matrix
  glPopMatrix();
}

//______________________________________________________________________________
template <typename TextureType>
RenderManager<TextureType>::RenderManager() :
  _renderer(nullptr),
  _window(nullptr),
  _glContext(nullptr),
  _renderScale(1.0, 1.0) {}

//______________________________________________________________________________
template <typename TextureType>
void RenderManager<TextureType>::Init()
{
  SDL_Init(SDL_INIT_EVERYTHING | SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK);
  TTF_Init();

  _window = SDL_CreateWindow(Title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    m_nativeWidth, m_nativeHeight,
    SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);

  _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);
  SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND);

  // init the gl context and a bunch of other stuff to enable GL
  if constexpr (std::is_same_v<TextureType, GLTexture>)
  {
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    _glContext = SDL_GL_CreateContext(_window);
    SDL_GL_MakeCurrent(_window, _glContext);

    // Enable vsync
    SDL_GL_SetSwapInterval(1);

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, m_nativeWidth, m_nativeHeight, 0, 0, 16);
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    SDL_RenderSetScale(_renderer, 1.0f, 1.0f);
  }

#ifndef _WIN32
  _sdlWindowFormat = SDL_GetWindowPixelFormat(_window);
#else
  _sdlWindowFormat = SDL_PIXELFORMAT_RGBA8888;
#endif
}

//______________________________________________________________________________
template <typename TextureType>
void RenderManager<TextureType>::Destroy()
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
template <typename TextureType>
void RenderManager<TextureType>::ProcessResizeEvent(const SDL_Event& event)
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
template <typename TextureType>
void RenderManager<TextureType>::Draw()
{
  for (int layer = 0; layer < (int)RenderLayer::NLayers; layer++)
  {
    if (_drawers[layer].camera)
    {
      _drawers[layer].textureDrawer.PerformDraw(_renderer, _drawers[layer].camera);
      _drawers[layer].primitiveDrawer.PerformDraw(_renderer, _drawers[layer].camera);
    }
  }
}

//______________________________________________________________________________
template <> void RenderManager<SDL_Texture>::Clear()
{
  SDL_RenderClear(_renderer);
}

//______________________________________________________________________________
template <> void RenderManager<GLTexture>::Clear()
{
  // clear previous render
  glClearColor(0.0, 0.0, 0.0, 1);
  glClear(GL_COLOR_BUFFER_BIT);

  // change display color
  glColor4f(1.0, 1.0, 1.0, 1.0);
}

//______________________________________________________________________________
template <> void RenderManager<SDL_Texture>::Present()
{
  SDL_RenderPresent(_renderer);
}

//______________________________________________________________________________
template <> void RenderManager<GLTexture>::Present()
{
  SDL_GL_SwapWindow(_window);
}




template class RenderManager<SDL_Texture>;
template class RenderManager<GLTexture>;
