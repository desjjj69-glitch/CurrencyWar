#include"SDL_Functions.h"
namespace MySDL {
	void DisplayWord(
		TTF_Font* &ft,
		SDL_Renderer*&rdr,
		const std::string& text,
		float x, float y,
		SDL_Texture*& texture,
		SDL_FRect& Rect,
		SDL_Color color,
		float multiply) {
		if (texture) {
			SDL_DestroyTexture(texture);
			texture = nullptr;
		}
		SDL_Surface* surface;
		surface = TTF_RenderText_Solid(ft, text.c_str(), 0, color);
		texture = SDL_CreateTextureFromSurface(rdr, surface);
		SDL_DestroySurface(surface);
		Rect = { x,y,static_cast<float>(texture->w) * multiply,static_cast<float>(texture->h) * multiply };
	}
	void DisplayTexture_DefaultSize(SDL_Renderer*& rdr,SDL_Texture*& tex, SDL_FRect& rect) {
		SDL_FRect drect = {
			rect.x,rect.y,static_cast<float>(tex->w),static_cast<float>(tex->h)
		};
		SDL_RenderTexture(rdr, tex, NULL, &drect);
	}
}