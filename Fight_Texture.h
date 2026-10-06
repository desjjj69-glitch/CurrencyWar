#pragma once
#include"SDL_Functions.h"
#include"BasicTexture.h"
#include"json.hpp"
#include<unordered_map>
#include<string>
#include<vector>
using json = nlohmann::json;
namespace Fight {
	class Fight_Texture:public BasicTexture {
	public:
		void Loadin(const std::filesystem::path&, SDL_Renderer*&)override;
		void Draw(SDL_Renderer*&, TTF_Font*&)override;
		Fight_Texture()=default;
		~Fight_Texture();
	private:
		bool uiloaded = false;
		bool jsloaded = false;
		void LoadinJson(const std::filesystem::path&)override;
		void LoadinPic(const std::filesystem::path&, SDL_Renderer*&)override;
		std::unordered_map<std::string, std::unordered_map<std::string, SDL_Texture*>> dynamic_ui_tex;
		std::unordered_map<std::string, std::vector<SDL_FRect>>ui_place;
		std::unordered_map<std::string, std::unordered_map<std::string, std::string>>dynamic_ui_path;//各种ui图标


		struct ActionListTexture {
			float startx=0, starty=0;
			float startw=0, starth=0;
			float endy=0;
			float extrax=0, extray=0;
			float extraw=0, extrah=0;
			float extraendx=0;
		}actionlist_place;
		std::vector<std::string>actor_name;
		std::unordered_map<std::string, SDL_Texture*>actor_tex;
		std::unordered_map<std::string, SDL_Texture*>extraactor_tex;
		void ReadActionListPosition(const json& js, ActionListTexture&);
		void Draw_Background(SDL_Renderer*&);
		void Draw_ActionList(SDL_Renderer*&);
	};
}