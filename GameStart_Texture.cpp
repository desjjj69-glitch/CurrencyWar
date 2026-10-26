#include"GameStart_Texture.h"
#include"json.hpp"
#include"Logger.h"
#include"GameStart_Logic.h"
#include<fstream>
#include<sstream>
using json = nlohmann::json;
using Logger::Log;
namespace GameStart {
	static bool PlaceInside(float x, float y, SDL_FRect rect) {
		return x >= rect.x && x <= rect.x + rect.w && y >= rect.y && y <= rect.y + rect.h;
	}
	static SDL_FRect ScaleRectCentered(const SDL_FRect& r, float sx, float sy) {
		SDL_FRect o{ 0,0,0,0 };
		o.w = r.w * sx;
		o.h = r.h * sy;
		o.x = r.x - (o.w - r.w) * 0.5f;
		o.y = r.y - (o.h - r.h) * 0.5f;
		return o;
	}
	// 等比 1.2：ScaleRectCentered(r, 1.2f, 1.2f)
	// 只横向：ScaleRectCentered(r, 1.2f, 1.0f)
	Texture::~Texture() {
		// ---------- UI 纹理：两层 map ----------
		for (auto& [key, mp] : ui_tex) {
			for (auto& [name, tex] : mp) {
				if (tex) {
					SDL_DestroyTexture(tex);
					tex = nullptr;
				}
			}
			mp.clear();
		}
		ui_tex.clear();

		// ---------- Boss 纹理 ----------
		for (auto& [key, tex] : boss_tex) {
			if (tex) {
				SDL_DestroyTexture(tex);
				tex = nullptr;
			}
		}
		boss_tex.clear();

		// ---------- Strategy 纹理 ----------
		for (auto& [key, tex] : strategy_tex) {
			if (tex) {
				SDL_DestroyTexture(tex);
				tex = nullptr;
			}
		}
		strategy_tex.clear();

		// ---------- 路径 / 位置（纯数据，clear 即可） ----------
		ui_path.clear();
		boss_path.clear();
		ui_place.clear();
	}
	void Texture::Loadin(const std::filesystem::path& gp, SDL_Renderer*& rdr) {
		LoadinJson(gp);
		LoadinPic(gp, rdr);
		mouse_place = { -1.f,-1.f };
	 }
	void Texture::Draw(SDL_Renderer*& rdr, TTF_Font*& ft) {
		float wx = 0.f, wy = 0.f;
		SDL_GetMouseState(&wx, &wy);
		if (!SDL_RenderCoordinatesFromWindow(rdr, wx, wy, &mouse_place.x, &mouse_place.y))
			mouse_place = { -1.f, -1.f };   // 转换失败视为不在任何控件上
		SDL_SetRenderDrawColor(rdr, 0, 0, 0, 255);
		SDL_RenderClear(rdr);
		switch (State.state) {
		case Logic::State::Waiting:
			Draw_Background(rdr);
			Draw_Boss(rdr);
			Draw_NextStep(rdr);
			break;
		case Logic::State::EnvironmentChoose:
			break;
		}
		
	}
	void Texture::LoadinJson(const std::filesystem::path& gp) {
		auto ParseRect = [](const std::string& s) ->SDL_FRect {
			std::istringstream iss(s);
			SDL_FRect rect{ 0,0,0,0 };
			iss >> rect.x >> rect.y >> rect.w >> rect.h;
			if (iss.fail())return { 0 ,0,0,0 };
			return rect;
			};
		std::ifstream json_gamestart(gp / "data" / "GameStart_Texture.json");
		std::ifstream json_enemy(gp / "data" / "Enemy.json");
		json js_gs, js_enemy;
		if (!json_enemy.is_open() || !json_gamestart.is_open()) {
			Log("", __func__, Logcode::OPENFAILED);
		}
		try {
			json_gamestart >> js_gs;
			json_enemy >> js_enemy;

			for (auto& [key, vec] : js_gs.items()) {
				if (vec.contains("place")) {
					if (vec["place"].is_array()) {
						for (auto& var : vec["place"]) {
							ui_place[key].push_back(ParseRect(var.get<std::string>()));
						}
					}
					else if (vec["place"].is_string()) {
						ui_place[key].push_back(ParseRect(vec["place"].get<std::string>()));
					}
					else {
						ui_place[key].push_back({0,0,0,0});
					}
				}
				if (vec.contains("image")) {
					if (vec["image"].is_array()) {
						for (auto& var : vec["image"]) {
							std::string name, path;
							if (var.contains("name") && var["name"].is_string()) {
								name = var["name"].get<std::string>();
							}
							if (var.contains("path") && var["path"].is_string()) {
								path = var["path"].get<std::string>();
							}
							ui_path[key][name] = path;
						}
					}
				}
			}
			if (js_enemy.contains("bosses") && js_enemy["bosses"].is_array()) {
				for (auto& b : js_enemy["bosses"]) {
					if (!b.is_object()) continue;
					if (!b.contains("name") || !b.contains("image")) continue;
					std::string name = b["name"].get<std::string>();   // "boss_1"
					std::string image = b["image"].get<std::string>();  // "boss_1.png"
					boss_path[name] = image;
				}
			}
		}
		catch (const std::exception& e) {
			Log(e.what(), __func__, Logcode::ERROR);
		}
	}
	void Texture::LoadinPic(const std::filesystem::path& gp, SDL_Renderer*& rdr) {
		std::filesystem::path gs_picp = gp / "assets" / "GameStart";
		std::filesystem::path enemy_picp = gp / "assets" / "Enemy";

		for (auto& [key, mp] : ui_path) {
			for (auto& [name, path] : mp) {
				auto tmp = IMG_LoadTexture(rdr, (gs_picp / path).string().c_str());
				ui_tex[key][name] = tmp;
			}
		}

		for (auto& [key, path] : boss_path) {
			auto tmp = IMG_LoadTexture(rdr, (enemy_picp / path).string().c_str());
			boss_tex[key] = tmp;
		}
	}
	void Texture::Draw_Background(SDL_Renderer*& rdr) {
		SDL_RenderTexture(rdr, ui_tex["background"]["Default"], NULL, &ui_place["background"][0]);
	}
	void Texture::Draw_Boss(SDL_Renderer*& rdr) {
		
		for (int i = 0;i< State.Boss_name.size();++i) {
			SDL_RenderTexture(rdr, boss_tex[State.Boss_name[i]], NULL, &ui_place["boss"][i]);
		}
	}
	void Texture::Draw_NextStep(SDL_Renderer*& rdr) {
		SDL_FRect rect = ui_place["nextstep"][0];
		if (PlaceInside(mouse_place.x, mouse_place.y, rect)) {
			rect=ScaleRectCentered(rect, 1.2, 1.2);
		}
		SDL_RenderTexture(rdr, ui_tex["nextstep"]["Default"], NULL, &rect);
	}
}