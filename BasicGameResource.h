#pragma once
#include<filesystem>
#include<SDL3/SDL.h>
#include<SDL3_image/SDL_image.h>
#include<SDL3_ttf/SDL_ttf.h>
#include<SDL3_mixer/SDL_mixer.h>
#include"BasicConfig.h"
#include"AudioManager.h"
class BasicGameResource {
public:
	SDL_Window* GameWindow=nullptr;
	SDL_Renderer* GameRenderer = nullptr;
	TTF_Font* GameFont = nullptr;
	std::filesystem::path GamePath;
	BasicConfig GameConfig;
	MIX_Mixer* GameMixer = nullptr;
	AudioManager Audio;
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