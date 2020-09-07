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

std::vector<GLDrawOperation> LetterCase::CreateStringField(const char* text, int fieldWidth, TextAlignment alignment, float lineHeight, float kerning)
{
  GLRenderString string;
  string.ratio = _fontSize / glyphs[0]->h();

  // stores last texture index for current line
  int lineBeginIndex = 0;

  // center aligns characters from x to y
  auto centerLine = [](std::vector<GLDrawOperation>& stringCharacters, int lineBegin, int lineEnd, float space)
  {
    for (int i = lineBegin; i <= lineEnd; i++)
    {
      // shift all characters towards the margin by half
      stringCharacters[i].x += (space / 2.0f);
    }
  };

  int lastCharacterIndex = 0;
  char c;
  for (int i = 0; text[i] != '\0'; i++)
  {
    lastCharacterIndex = static_cast<int>(string.textures.size());

    // deal with linebreak
    if (text[i] == '\n')
    {
      if (alignment == TextAlignment::Centered)
      {
        centerLine(string.textures, lineBeginIndex, lastCharacterIndex, fieldWidth - string.x);
      }

      string.x = 0;
      string.y += _fontSize * lineHeight;
      lineBeginIndex = lastCharacterIndex - 1;
    }

    // clamp to ' ' (96)
    c = text[i] < ' ' ? ' ' : text[i];

    // line wrap
    if (c != ' ' && abs((int)string.x) >= fieldWidth)
    {
      if (alignment == TextAlignment::Centered)
      {
        centerLine(string.textures, lineBeginIndex, lastCharacterIndex, fieldWidth - string.x);
      }

      string.x = 0;
      string.y += _fontSize * lineHeight;
      lineBeginIndex = lastCharacterIndex - 1;
    }

    // shift to match our letter case indices
    c -= ' ';

    string.textures.push_back({ string.x, string.y, static_cast<float>(_fontSize), &glyphs[c] });

    string.x += glyphs[c]->w() * (_fontSize / glyphs[c]->h()) * kerning;
  }

  // align last line
  if (alignment == TextAlignment::Centered)
  {
    if(lineBeginIndex != lastCharacterIndex - 1)
      centerLine(string.textures, lineBeginIndex, lastCharacterIndex, fieldWidth - string.x);

    // align all characters to the beginning of text field
    for (int i = 0; i < string.textures.size(); i++)
    {
      string.textures[i].x -= (static_cast<float>(fieldWidth) / 2.0f);
      string.textures[i].y -= ((string.y + (_fontSize * lineHeight)) / 2.0f);
    }
  }

  return string.textures;
}
