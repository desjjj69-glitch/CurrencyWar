#include"WaitingForFight_EnemyTexture.h"
#include"EnemyObject.h"
#include<fstream>
#include"Logger.h"
#include"json.hpp"
using json = nlohmann::json;
using Logger::Log;
namespace WaitingForFight {
	void WaitingForFight_EnemyTexture::LoadinJson(const std::filesystem::path& gp) {
		//Log(std::to_string(Object::Enemy_DefaultSetting.empty()), "Debug");
		for (auto& [key, value] : Object::Enemy_DefaultSetting) {
			reflect[key] = key + ".png";
			reflect[key + "_Head"] = key + "_Head.png";
			//Log(key+reflect[key],"Debug");
		}
	}
	void WaitingForFight_EnemyTexture::LoadinPic(const std::filesystem::path& gp, SDL_Renderer*& rdr) {
		std::filesystem::path picp = gp / "assets" / "Enemy";
		for (auto& [key, var] : reflect) {
			auto tmp = IMG_LoadTexture(rdr, (picp / var).string().c_str());
			Log(key, __FUNCSIG__, tmp ? Logcode::OK : Logcode::OPENFAILED);
			tex[key] = tmp;
		}
	}
	void WaitingForFight_EnemyTexture::Loadin(const std::filesystem::path& gp, SDL_Renderer*& rdr) {

		if (!jsin) {
			LoadinJson(gp);
			jsin = true;
		}
		if (!picin) {
			LoadinPic(gp, rdr);
			picin = true;
		}
	}
	void WaitingForFight_EnemyTexture::Draw(SDL_Renderer*&, TTF_Font*&) {

	}
	WaitingForFight_EnemyTexture::~WaitingForFight_EnemyTexture() {
		for (auto& [key, value] : tex) {
			SDL_DestroyTexture(value);
		}
		tex.clear();
		picin = false;
	}
	SDL_Texture* WaitingForFight_EnemyTexture::GetEnemyTexture(const std::string& name) {
		return tex[name];
	}
}