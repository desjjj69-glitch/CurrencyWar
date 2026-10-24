#include"WaitingForFight_MineralTexture.h"
#include<fstream>
#include"Logger.h"
#include"json.hpp"
using json = nlohmann::json;
using Logger::Log;
namespace WaitingForFight {
	void WaitingForFight_MineralTexture::LoadinJson(const std::filesystem::path& gp) {
		std::ifstream configin(gp / "data" / "Mineral_Texture.json");
		Log("", __func__, configin.is_open() ? Logcode::OK : Logcode::OPENFAILED);
		try {
			json js;
			configin >> js;
			if (js.contains("Mineral") && js["Mineral"].is_array()) {
				for (auto& var : js["Mineral"]) {
					reflect[var.value("Name", "Unknown")] = var.value("Path", "Unknown");
				}
			}
		}
		catch (const json::parse_error&) {
			Log("WaitingForFight_MT", __func__, Logcode::PARSEERROR);
		}
		catch (const std::exception& e) {
			Log(e.what(), __func__, Logcode::ERROR);
		}
	}
	void WaitingForFight_MineralTexture::LoadinPic(const std::filesystem::path& gp, SDL_Renderer*& rdr) {
		std::filesystem::path picp = gp / "assets" / "Mineral";
		for (auto& [key, var] : reflect) {
			auto tmp = IMG_LoadTexture(rdr, (picp / var).string().c_str());
			Log(key, __func__, tmp ? Logcode::OK : Logcode::OPENFAILED);
			tex[key] = tmp;
		}
	}
	void WaitingForFight_MineralTexture::Loadin(const std::filesystem::path& gp, SDL_Renderer*& rdr) {

		if (!jsin) {
			LoadinJson(gp);
			jsin = true;
		}
		if (!picin) {
			LoadinPic(gp, rdr);
			picin = true;
		}
	}
	void WaitingForFight_MineralTexture::Draw(SDL_Renderer*&, TTF_Font*&) {

	}
	WaitingForFight_MineralTexture::~WaitingForFight_MineralTexture() {
		for (auto& [key, value] : tex) {
			SDL_DestroyTexture(value);
		}
		tex.clear();
		picin = false;
	}
	SDL_Texture* WaitingForFight_MineralTexture::GetMineralTexture(const std::string& name) {
		auto it = tex.find(name);
		return it == tex.end() ? nullptr : it->second;
	}
	SDL_Texture* WaitingForFight_MineralTexture::GetMineralTexture(MineralType type) {
		return GetMineralTexture(MineralTypeName(type));
	}
}
