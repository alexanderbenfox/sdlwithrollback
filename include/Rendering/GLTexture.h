#pragma once

#if defined(_WIN32)
#if defined(APIENTRY)
#undef APIENTRY
#endif
#include <windows.h>
#include <GL/glew.h>

#else
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#endif

#include <SDL2/SDL_image.h>
#include <string>

static GLenum GetScaleQuality()
{
  const char* hint = SDL_GetHint(SDL_HINT_RENDER_SCALE_QUALITY);

  if (!hint || *hint == '0' || SDL_strcasecmp(hint, "nearest") == 0) {
    return GL_NEAREST;
  }
  else {
    return GL_LINEAR;
  }
}

struct GLTexImageParams
{
  GLint internalFormat;
  GLenum textureFormat;
  GLenum type;
};

static GLTexImageParams SDL_PixelFormatToGL(Uint32 format)
{
  GLTexImageParams params;
  switch (format)
  {
  case SDL_PIXELFORMAT_ARGB8888:
    params.internalFormat = GL_RGBA8;
    params.textureFormat = GL_BGRA;
    params.type = GL_UNSIGNED_INT_8_8_8_8_REV;
    break;
  case SDL_PIXELFORMAT_YV12:
  case SDL_PIXELFORMAT_IYUV:
    params.internalFormat = GL_LUMINANCE;
    params.textureFormat = GL_LUMINANCE;
    params.type = GL_UNSIGNED_BYTE;
    break;
  default:
    break;
  }
  return params;
}


// gl texture class for showing textures in debug ui
class GLTexture
{
public:
  //! default constructor
  GLTexture() = default;
  //! constructor with specific texture id
  GLTexture(GLint textureId) : _textureId(textureId) {}

  //! rvalue copy constructor
  GLTexture(GLTexture&& other) noexcept;
  //! rvalue assignment operator
  GLTexture& operator=(GLTexture&& other) noexcept;
  //! deconstuctor removes texture from gl context
  ~GLTexture() noexcept;

  //! Load a file in memory and create an OpenGL texture.
  void LoadFromFile(const std::string& fileName);
  //!
  void LoadFromSurface(SDL_Surface* surface);

  void CreateEmpty(int width, int height, Uint32 format);

  void Update(void* pixels);

  //! Get texture id
  GLuint ID() const { return _textureId; }
  SDL_BlendMode BlendMode() const { return _blendMode; }
  int w() { return _w; }
  int h() { return _h; }

private:
  //! delete copy constructor and assignment operator
  GLTexture(const GLTexture& other) = delete;
  GLTexture& operator=(const GLTexture& other) = delete;
  
  //! Sets GL parameters based on SDL_Surface
  void SetTextureParameters(SDL_Surface* textureData);

  // width and height of texture in pixels
  int _w, _h;
  // specifies name of texture as it is bound by the open gl context
  GLuint _textureId;
  // Specifies the data type of the pixel data
  // GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT, GL_UNSIGNED_INT, GL_INT, GL_HALF_FLOAT, GL_FLOAT, GL_UNSIGNED_SHORT_5_6_5, GL_UNSIGNED_SHORT_4_4_4_4, GL_UNSIGNED_SHORT_5_5_5_1,
  // GL_UNSIGNED_INT_2_10_10_10_REV, GL_UNSIGNED_INT_10F_11F_11F_REV, GL_UNSIGNED_INT_5_9_9_9_REV, GL_UNSIGNED_INT_24_8, and GL_FLOAT_32_UNSIGNED_INT_24_8_REV.
  GLenum _type;
  // Specifies the format of the pixel data
  // GL_RED, GL_RED_INTEGER, GL_RG, GL_RG_INTEGER, GL_RGB, GL_RGB_INTEGER, GL_RGBA, GL_RGBA_INTEGER, GL_DEPTH_COMPONENT, GL_DEPTH_STENCIL, GL_LUMINANCE_ALPHA, GL_LUMINANCE, and GL_ALPHA.
  GLenum _textureFormat;
  // Specifies the number of color components in the texture
  GLuint _internalFormat;


  //___________________________________________
  // SDL PARAMETERS
  //___________________________________________

  SDL_BlendMode _blendMode;
};
