#pragma once
#include"BasicTexture.h"
#include"SDL_Functions.h"
namespace GameStart {
	class Texture : public BasicTexture {
	public:
		void Loadin(const std::filesystem::path&, SDL_Renderer*&)override;
		void Draw(SDL_Renderer*&, TTF_Font*&)override;
		~Texture();
	private:
		void LoadinJson(const std::filesystem::path&) override;
		void LoadinPic(const std::filesystem::path&, SDL_Renderer*&)override;
	};
}