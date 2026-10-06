#pragma once
#include"BasicTexture.h"
#include"FightResult_Logic.h"
namespace FightResult {
	class FightResult_Texture :public BasicTexture {
	public:
		virtual void Loadin(const std::filesystem::path&, SDL_Renderer*&)override;
		FightResult_Texture()=default;
		~FightResult_Texture();
		void Draw(SDL_Renderer*&, TTF_Font*&);
	private:
		bool jsloaded = false;
		bool uiloaded = false;
		void LoadinJson(const std::filesystem::path&)override;
		void LoadinPic(const std::filesystem::path&, SDL_Renderer*&)override;
	};
}