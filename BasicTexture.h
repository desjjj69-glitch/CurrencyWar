#pragma once
#include<SDL3/SDL.h>
#include<SDL3_image/SDL_image.h>
#include<SDL3_ttf/SDL_ttf.h>
#include<filesystem>
class BasicTexture {
public:
	virtual void Loadin(const std::filesystem::path&, SDL_Renderer*&) = 0;
	virtual void Draw(SDL_Renderer*&,TTF_Font*&) = 0;
	BasicTexture() = default;
	BasicTexture(const BasicTexture&) = delete;
	BasicTexture& operator=(const BasicTexture&) = delete;
	BasicTexture(BasicTexture&&) = delete;
	BasicTexture& operator=(BasicTexture&&) = delete;
	virtual ~BasicTexture() = default;
private:
	virtual void LoadinJson(const std::filesystem::path&) = 0;
	virtual void LoadinPic(const std::filesystem::path&, SDL_Renderer*&) = 0;
};