#include"BasicGameResource.h"
#include"Logger.h"
using Logger::Log;
void BasicGameResource::init() {
	GamePath = SDL_GetBasePath();
	GameConfig.LoadConfig(GamePath);
	Log("SDL_Init",__func__, SDL_Init(GameConfig.GameMode)?Logcode::OK:Logcode::ERROR);
	
	int w = static_cast<int>(GameConfig.WindowW);
	int h = static_cast<int>(GameConfig.WindowH);
	SDL_Rect usable{ 0, 0, 0, 0 };
	if (SDL_GetDisplayUsableBounds(SDL_GetPrimaryDisplay(), &usable)) {
		
	}
	GameConfig.WindowMode |= SDL_WINDOW_RESIZABLE;
	GameWindow = SDL_CreateWindow(
		GameConfig.GameName.c_str(),
		static_cast<int>(w),
		static_cast<int>(h),
		GameConfig.WindowMode
	);
	Log("Window", __func__, GameWindow == nullptr?Logcode::CREATEFAILED:Logcode::OK);
	GameRenderer = SDL_CreateRenderer(GameWindow, NULL);
	if (usable.w > 0 && usable.h > 0) {
		SDL_SetWindowPosition(GameWindow, usable.x+100, usable.y+100);
	}
	SDL_SetRenderLogicalPresentation(
		GameRenderer,
		2560,
		1600,
		SDL_LOGICAL_PRESENTATION_LETTERBOX
	);
	Log("Renderer", __func__, GameRenderer == nullptr ? Logcode::CREATEFAILED : Logcode::OK);
	TTF_Init();
	GameFont = TTF_OpenFont((GamePath / "assets" / GameConfig.FontName).string().c_str(), GameConfig.FontSize);
	Log("Font", __func__, GameFont == nullptr ? Logcode::CREATEFAILED : Logcode::OK);
	SDL_SetRenderDrawBlendMode(GameRenderer, SDL_BLENDMODE_BLEND);
	FrameTime = GameConfig.FrameTime;
	if (!MIX_Init()) {
		Log("Fail to init mix", __func__, Logcode::ERROR);
	}
	GameMixer=MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
	Audio.Init(GamePath, GameMixer);
}
void BasicGameResource::quit() {
	Audio.Shutdown();
	if (GameMixer) {
		MIX_DestroyMixer(GameMixer);
		MIX_Quit();
	}
	if (GameFont) {
		TTF_CloseFont(GameFont);
		TTF_Quit();
	}
	if (GameRenderer) {
		SDL_DestroyRenderer(GameRenderer);
	}
	if (GameWindow) {
		SDL_DestroyWindow(GameWindow);
	}
	SDL_Quit();
}
BasicGameResource::~BasicGameResource() {
	quit();
	GameFont = nullptr;
	GameRenderer = nullptr;
	GameWindow = nullptr;
}
BasicGameResource BasicGameRes;