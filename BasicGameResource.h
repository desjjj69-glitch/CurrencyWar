#pragma once
#include<filesystem>
#include<SDL3/SDL.h>
#include<SDL3_image/SDL_image.h>
#include<SDL3_ttf/SDL_ttf.h>
#include"BasicConfig.h"
class BasicGameResource {
public:
	SDL_Window* GameWindow=nullptr;
	SDL_Renderer* GameRenderer = nullptr;
	TTF_Font* GameFont = nullptr;
	std::filesystem::path GamePath;
	BasicConfig GameConfig;
	Uint32 FrameTime = 30;
	void init();
	void quit();

	BasicGameResource() = default;
	BasicGameResource(const BasicGameResource&) = delete;
	BasicGameResource& operator=(const BasicGameResource&) = delete;
	BasicGameResource(BasicGameResource&&) = delete;
	BasicGameResource& operator=(BasicGameResource&&) = delete;
	~BasicGameResource();
};
extern BasicGameResource BasicGameRes;