#pragma once
#include"SDL_Functions.h"
#include"BasicTexture.h"
#include"WaitingForFight_InteractLogic.h"
#include"WaitingForFight_CharacterTexture.h"
#include"WaitingForFight_EnemyTexture.h"
#include"WaitingForFight_EquipmentTexture.h"
#include"WaitingForFight_MineralTexture.h"
#include"BasicObject.h"
#include<unordered_map>
#include<string>
#include<vector>
namespace WaitingForFight {
	class WaitingForFightTexture :public BasicTexture {//统一管理所有备战阶段的绘制
	public:
		void Loadin(const std::filesystem::path&, SDL_Renderer*&)override;
		void Draw(SDL_Renderer*&,TTF_Font*&)override;
		~WaitingForFightTexture();
	private:
		WaitingForFight_CharacterTexture character_tex;
		WaitingForFight_EnemyTexture enemy_tex;
		WaitingForFight_EquipmentTexture equipment_tex;
		WaitingForFight_MineralTexture mineral_tex;
		bool Loaded = 0;
		bool texLoaded = 0;
		void LoadinJson(const std::filesystem::path&)override;
		void LoadinPic(const std::filesystem::path&, SDL_Renderer*&)override;
		std::unordered_map<std::string, std::vector<SDL_Texture*> > tex;
		std::unordered_map<std::string, std::vector<std::string> > reflect;
		std::unordered_map<std::string, std::vector<SDL_FRect>> place;
		std::unordered_map<std::string, std::vector<SDL_FRect>>picsize;
		
		std::unordered_map<std::string, SDL_Color>word_color;
		std::unordered_map<std::string, float>word_multiply;
		std::unordered_map<std::string, SDL_FRect>word_place;
		std::unordered_map<std::string, SDL_Texture*>word_tex;
		void Word_Simplepara(SDL_Renderer*&, TTF_Font*&, const std::string&, const std::string&);//text,key
		void Draw_Background(SDL_Renderer*&);
		void Draw_QuitButton(SDL_Renderer*&);
		void Draw_Blood(SDL_Renderer*&, TTF_Font*&);
		void Draw_Stage(SDL_Renderer*&, TTF_Font*&);
		void Draw_CharacterOnBattleFieldLimit(SDL_Renderer*&, TTF_Font*&);
		void Draw_ContinueWin(SDL_Renderer*&, TTF_Font*&);
		void Draw_BuyExp(SDL_Renderer*&, TTF_Font*&);
		void Draw_ShopButton(SDL_Renderer*&, TTF_Font*&);
		void Draw_StartFightButton(SDL_Renderer*&, TTF_Font*&);
		void Draw_Shop(SDL_Renderer*&, TTF_Font*&);
		void Draw_Bench(SDL_Renderer*&);
		void Draw_Mineral(SDL_Renderer*&);
		void Draw_FrontBattleField(SDL_Renderer*&);
		void Draw_BackBattleField(SDL_Renderer*&);
		void Draw_holding(SDL_Renderer*& rdr);
		void Draw_Sell(SDL_Renderer*&,TTF_Font*&);
		void Draw_Character(SDL_Renderer*&, std::unique_ptr<BasicObject>&,SDL_FRect&,int signal=0);
		void Draw_EnemyPreview(SDL_Renderer*&, TTF_Font*&);
		void Draw_GameStartPreview(SDL_Renderer*&, TTF_Font*&);
	};
}
