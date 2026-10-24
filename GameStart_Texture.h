#pragma once
#include"BasicTexture.h"
#include"SDL_Functions.h"
#include<unordered_map>
#include<string>
#include<vector>
namespace GameStart {
	class Texture : public BasicTexture {
	public:
		void Loadin(const std::filesystem::path&, SDL_Renderer*&)override;
		void Draw(SDL_Renderer*&, TTF_Font*&)override;
		~Texture();
		SDL_FPoint mouse_place{ -1.f,-1.f };
	private:
		void LoadinJson(const std::filesystem::path&) override;
		void LoadinPic(const std::filesystem::path&, SDL_Renderer*&)override;

		std::unordered_map<std::string, std::unordered_map<std::string, SDL_Texture*>> ui_tex;
		std::unordered_map<std::string, std::unordered_map<std::string, std::string>>ui_path;
		std::unordered_map<std::string, std::vector<SDL_FRect>> ui_place;

		std::unordered_map<std::string, SDL_Texture*>boss_tex;
		std::unordered_map<std::string, std::string>boss_path;
		
		std::unordered_map<std::string, SDL_Texture*> strategy_tex;

		void Draw_Background(SDL_Renderer*&);
		void Draw_Boss(SDL_Renderer*& rdr);
	};
}