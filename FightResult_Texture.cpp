#include"FightResult_Texture.h"
namespace FightResult {
	void FightResult_Texture::Loadin(const std::filesystem::path& gp, SDL_Renderer*& rdr) {
		if (!jsloaded) {
			LoadinJson(gp);
			jsloaded = true;
		}
		if (!uiloaded) {
			LoadinPic(gp, rdr);
			uiloaded = true;
		}
	}
	void FightResult_Texture::LoadinJson(const std::filesystem::path& gp) {
		
	}
	void FightResult_Texture::LoadinPic(const std::filesystem::path& gp, SDL_Renderer*& rdr) {

	}
	void FightResult_Texture::Draw(SDL_Renderer*& rdr, TTF_Font*& ft) {
		SDL_SetRenderDrawColor(rdr, 0, 0, 0, 255);
		SDL_RenderClear(rdr);
	}
	FightResult_Texture::~FightResult_Texture() {

	}
}