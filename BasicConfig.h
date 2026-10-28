#pragma once
#include<filesystem>
#include<fstream>
#include<string>
#include<SDL3/SDL.h>
#include<SDL3_image/SDL_image.h>
#include<SDL3_ttf/SDL_ttf.h>
class BasicConfig {
public:
	void LoadConfig(const std::filesystem::path&);
	bool SetGameSaved(const std::filesystem::path&, int value);
	Uint32 GameMode = SDL_INIT_AUDIO | SDL_INIT_VIDEO;
	std::string GameName = "CurrencyWar";
	float WindowW = 2560;
	float WindowH = 1600;
	Uint64 WindowMode = SDL_WINDOW_FULLSCREEN;
	std::string FontName = "simhei.ttf";
	float FontSize = 32;
	Uint32 FrameTime = 30;
	int GameSaved = 0;
};