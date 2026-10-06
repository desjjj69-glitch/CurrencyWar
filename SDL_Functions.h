#pragma once
#include<SDL3/SDL.h>
#include<SDL3_image/SDL_image.h>
#include<SDL3_ttf/SDL_ttf.h>
#include<string>
namespace MySDL {
	void DisplayWord(
		TTF_Font*&,
		SDL_Renderer*&,
		const std::string&,
		float, float,
		SDL_Texture*&,
		SDL_FRect&,
		SDL_Color,
		float);//文本/位置/接受材质/接受位置/颜色/字体大小
	void DisplayTexture_DefaultSize(SDL_Renderer*&,SDL_Texture*&, SDL_FRect&);//只要求读取json中的x,y并传入即可
}