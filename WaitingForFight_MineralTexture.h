#pragma once
#include"BasicTexture.h"
#include"MineralType.h"
#include<unordered_map>
#include<string>
namespace WaitingForFight {
	class WaitingForFight_MineralTexture :public BasicTexture {
	public:
		virtual void Loadin(const std::filesystem::path&, SDL_Renderer*&)override;
		virtual void Draw(SDL_Renderer*&, TTF_Font*&)override;
		SDL_Texture* GetMineralTexture(const std::string&);
		SDL_Texture* GetMineralTexture(MineralType);
		~WaitingForFight_MineralTexture()override;
	private:
		bool jsin = false;
		bool picin = false;
		virtual void LoadinJson(const std::filesystem::path&)override;
		virtual void LoadinPic(const std::filesystem::path&, SDL_Renderer*&)override;
		std::unordered_map<std::string, SDL_Texture*> tex;
		std::unordered_map<std::string, std::string> reflect;
	};
}
