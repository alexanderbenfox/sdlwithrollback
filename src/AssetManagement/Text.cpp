#include "AssetManagement/Text.h"
#include "GameManagement.h"

TextResource::TextResource(Resource<TTF_Font> font, const char* text, SDL_Color color)
{
  SDL_Surface* textSurf = TTF_RenderText_Solid(font.Get(), text, color);
  _info.mHeight = textSurf->h;
  _info.mWidth = textSurf->w;

  _resource = std::shared_ptr<SDL_Texture>(SDL_CreateTextureFromSurface(GameManager::Get().GetRenderer(), textSurf), SDL_DestroyTexture);

  if (_resource) _loaded = true;

  SDL_FreeSurface(textSurf);
  textSurf = NULL;
}

void TextResource::Load()
{
  
}