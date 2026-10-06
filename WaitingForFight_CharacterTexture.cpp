#include"WaitingForFight_CharacterTexture.h"
#include<fstream>
#include"Logger.h"
#include"json.hpp"
using json = nlohmann::json;
using Logger::Log;
namespace WaitingForFight {
	void WaitingForFight_CharacterTexture::LoadinJson(const std::filesystem::path& gp) {
		std::ifstream configin(gp / "data" / "BasicCharacterSettings.json");
		Log("", __func__, configin.is_open() ? Logcode::OK : Logcode::OPENFAILED);
		try {
			json js;
			configin >> js;
			if (js.contains("Characters") && js["Characters"].is_array()) {
				for (auto& var : js["Characters"]) {
					reflect[var.value("Name", "Unknown")] = var.value("Path", "Unknown");
				}
			}
		}
		catch (const json::parse_error&) {
			Log("WaitingForFight_CT", __func__, Logcode::PARSEERROR);
		}
		catch (const std::exception& e) {
			Log(e.what(), __func__, Logcode::ERROR);
		}
	}
	void WaitingForFight_CharacterTexture::LoadinPic(const std::filesystem::path& gp, SDL_Renderer*& rdr) {
		std::filesystem::path picp = gp / "assets" / "Character";
		for (auto& [key, var] : reflect) {
			auto tmp = IMG_LoadTexture(rdr, (picp / var).string().c_str());
			Log(key, __func__, tmp ? Logcode::OK : Logcode::OPENFAILED);
			tex[key] = tmp;
		}
	}
	void WaitingForFight_CharacterTexture::Loadin(const std::filesystem::path& gp, SDL_Renderer*& rdr) {

		if (!jsin) {
			LoadinJson(gp);
			jsin = true;
		}
		if (!picin) {
			LoadinPic(gp, rdr);
			picin = true;
		}
	}
	void WaitingForFight_CharacterTexture::Draw(SDL_Renderer*&,TTF_Font*&) {

	}
	WaitingForFight_CharacterTexture::~WaitingForFight_CharacterTexture() {
		for (auto& [key, value] : tex) {
			SDL_DestroyTexture(value);
		}
		tex.clear();
		picin = false;
	}
	SDL_Texture* WaitingForFight_CharacterTexture::GetCharacterTexture(const std::string& name) {
		return tex[name];
	}
}
