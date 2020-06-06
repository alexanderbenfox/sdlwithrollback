#include "AssetManagement/LetterCase.h"
#include "Rendering/GLTexture.h"

LetterCase::LetterCase() : _fontSize(0)
{
  for (int i = 0; i < alphabetSize; ++i)
  {
    glyphs[i] = Resource<GLTexture>();
  }
}

LetterCase::LetterCase(TTF_Font* font, size_t size) : _fontSize(size)
{
  SDL_Surface* surf;

  // This could easily be modified for utf-8 by using utf-8 char codes 
  // and TTF_RenderGlyph_Blended() instead. 
  // Just get the ASCII latin1 charset
  char s[] = " ";
  for (char i = 0; i < alphabetSize; ++i)
  {
    s[0] = i + ' ';
    surf = TTF_RenderText_Blended(font, s, SDL_Color{ 255, 255, 255 });
    surf->refcount++; // SDL2: prevent segfault on free
    glyphs[i] = Resource<GLTexture>(std::shared_ptr<GLTexture>(new GLTexture));
    glyphs[i].Get()->LoadFromSurface(surf);
    SDL_FreeSurface(surf);
  }
}

LetterCase::~LetterCase()
{
  /*if (glyphs[0] != nullptr)
  {
    for (int i = 0; i < alphabetSize; ++i)
    {
      delete glyphs[i];
    }
  }*/
}

std::vector<GLDrawOperation> LetterCase::CreateStringField(const char* text, int fieldWidth, float lineHeight, float kerning)
{
  GLRenderString string;
  string.ratio = _fontSize / glyphs[0]->h();

  char c;
  for (int i = 0; text[i] != '\0'; i++)
  {
    // deal with linebreak
    if (text[i] == '\n')
    {
      string.x = 0;
      string.y += _fontSize * lineHeight;
    }

    // clamp to ' ' (96)
    c = text[i] < ' ' ? ' ' : text[i];

    // line wrap
    if (c != ' ' && abs((int)string.x) >= fieldWidth)
    {
      string.x = 0;
      string.y += _fontSize * lineHeight;
    }

    // shift to match our letter case indices
    c -= ' ';

    string.textures.push_back({ string.x, string.y, static_cast<float>(_fontSize), &glyphs[c] });

    string.x += glyphs[c]->w() * (_fontSize / glyphs[c]->h()) * kerning;
  }
  return string.textures;
}
