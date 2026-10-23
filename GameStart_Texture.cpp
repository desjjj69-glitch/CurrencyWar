#include"GameStart_Texture.h"
#include"json.hpp"
#include"Logger.h"
#include"GameStart_Logic.h"
#include<fstream>
#include<sstream>
using json = nlohmann::json;
using Logger::Log;
namespace GameStart {
	Texture::~Texture() {

	}
	void Texture::Loadin(const std::filesystem::path&, SDL_Renderer*&) {

	 }
	void Texture::Draw(SDL_Renderer*&, TTF_Font*&) {

	}
	void Texture::LoadinJson(const std::filesystem::path& gp) {
		std::ifstream json_gamestart(gp / "data" / "GameStart_Texture.json");
		std::ifstream json_enemy(gp / "data" / "Enemy.json");
		json js_gs, js_enemy;
		if (!json_enemy.is_open() || !json_gamestart.is_open()) {
			Log("", __func__, Logcode::OPENFAILED);
		}
		try {
			json_gamestart >> js_gs;
			json_enemy >> js_enemy;
		}
		catch (const std::exception& e) {
			Log(e.what(), __func__, Logcode::ERROR);
		}
	}
	void Texture::LoadinPic(const std::filesystem::path&, SDL_Renderer*&) {

	}
}