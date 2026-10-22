#pragma once
#include"BasicTexture.h"
#include"FightResult_Logic.h"
#include<string>
#include<unordered_map>
namespace FightResult {
	class FightResult_Texture :public BasicTexture {
	public:
		virtual void Loadin(const std::filesystem::path&, SDL_Renderer*&)override;
		FightResult_Texture()=default;
		~FightResult_Texture();
		void Draw(SDL_Renderer*&, TTF_Font*&);
		void Init();
	private:
		SDL_FPoint mouse_place{ -1.f,-1.f };
		bool jsloaded = false;
		bool uiloaded = false;
		std::unordered_map<std::string, std::vector<SDL_Texture*>> ui_tex;
		std::unordered_map<std::string, std::vector<SDL_FRect>> ui_place;
		std::unordered_map<std::string, std::vector<std::string>> ui_path;
		void LoadinJson(const std::filesystem::path&)override;
		void LoadinPic(const std::filesystem::path&, SDL_Renderer*&)override;

		void Draw_Background(SDL_Renderer*&, TTF_Font*&);
		void Draw_QuitButton(SDL_Renderer*&, TTF_Font*&);
	};
}