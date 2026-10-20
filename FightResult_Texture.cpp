#include"FightResult_Texture.h"
#include<fstream>
#include<sstream>
#include"json.hpp"
#include"Logger.h"
#include<string>
#include<vector>
using Logger::Log;
using json = nlohmann::json;
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
		ui_path.clear();
		ui_place.clear();
		std::ifstream configin(gp / "data" / "FightResult_Texture.json");
		json js;
		try {
			configin >> js;
		}
		catch (const std::exception& e) {
			Log(e.what(), __func__, Logcode::ERROR);
			return;
		}
		auto ParseRect = [](const std::string& s)->SDL_FRect{
			std::istringstream iss(s);
			SDL_FRect rect{ 0.0f, 0.0f, 0.0f, 0.0f };
			iss >> rect.x >> rect.y >> rect.w >> rect.h;
			if (iss.fail()) {
				Log( s, "ParseRect", Logcode::PARSEERROR);
			}
			return rect;
			};
		for (auto& [key,value] : js.items()) {
			if (value.is_array()) {
				for (auto& var : value) {
					ui_path[key].push_back(
						(var.contains("image") && var["image"].is_string())
						? var["image"].get<std::string>() : std::string{});
					ui_place[key].push_back(
						(var.contains("place") && var["place"].is_string())
						? ParseRect(var["place"].get<std::string>())
						: SDL_FRect{ 0,0,0,0 });
				}
			}
		}
	}
	void FightResult_Texture::LoadinPic(const std::filesystem::path& gp, SDL_Renderer*& rdr) {
		std::filesystem::path picp = gp / "assets" / "FightResult";
		
		for (auto& [key, vec] : ui_path) {
			for (auto& var : vec) {
				try {
					auto tmp = IMG_LoadTexture(rdr, (picp / var).string().c_str());
					if (tmp == nullptr) {
						throw std::runtime_error(
							std::string("IMG_LoadTexture failed: ") + SDL_GetError()
						);
					}
					Log(key + "+" + var, __func__, Logcode::OK);
					ui_tex[key].push_back(tmp);
					}
				catch (const std::exception& e) {
					ui_tex[key].push_back(nullptr);
					Log(e.what(), __func__, Logcode::OPENFAILED);
				}
			}
		}
	}
	void FightResult_Texture::Draw(SDL_Renderer*& rdr, TTF_Font*& ft) {
		SDL_SetRenderDrawColor(rdr, 0, 0, 0, 255);
		SDL_RenderClear(rdr);
		Draw_Background(rdr, ft);
	}
	FightResult_Texture::~FightResult_Texture() {
		for (auto& pair : ui_tex) {
			for (SDL_Texture* tex : pair.second) {
				if (tex) {
					SDL_DestroyTexture(tex);
				}
			}
		}
		ui_tex.clear();
	}
	void FightResult_Texture::Draw_Background(SDL_Renderer*& rdr, TTF_Font*& ft) {
		SDL_RenderTexture(rdr, ui_tex["background"][0], NULL, &ui_place["background"][0]);
	}
}