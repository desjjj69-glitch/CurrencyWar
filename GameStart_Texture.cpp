#include"GameStart_Texture.h"
#include"json.hpp"
#include"Logger.h"
#include"GameStart_Logic.h"
#include<fstream>
#include<sstream>
#include<algorithm>
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
	static SDL_FRect FitRectCentered(const SDL_FRect& box, float w, float h) {
		if (w <= 0.f || h <= 0.f) return box;
		const float scale = std::min(box.w / w, box.h / h);
		const float dw = w * scale;
		const float dh = h * scale;
		return { box.x + (box.w - dw) * 0.5f, box.y + (box.h - dh) * 0.5f, dw, dh };
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
			Draw_Environment(rdr);
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
		std::filesystem::path strategy_picp = gp / "assets" / "Strategy";

		for (auto& [key, mp] : ui_path) {
			const bool isStrategy = (key == "strategy_icon");
			for (auto& [name, path] : mp) {
				auto tmp = IMG_LoadTexture(rdr, ((isStrategy ? strategy_picp : gs_picp) / path).string().c_str());
				if (isStrategy) {
					strategy_tex[name] = tmp;
				}
				else {
					ui_tex[key][name] = tmp;
				}
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
	void Texture::Draw_Environment(SDL_Renderer*& rdr) {
		//背景
		auto bg = ui_tex.find("background_environment");
		if (bg != ui_tex.end()) {
			auto def = bg->second.find("Default");
			auto placeIt = ui_place.find("background_environment");
			if (def != bg->second.end() && def->second && placeIt != ui_place.end() && !placeIt->second.empty()) {
				SDL_RenderTexture(rdr, def->second, NULL, &placeIt->second[0]);
			}
		}

		auto envIt = ui_place.find("environment");
		//策略图标：按抽到的 id 取图，等比缩放并居中到对应卡片矩形
		if (envIt != ui_place.end()) {
			for (size_t i = 0; i < State.environment_strategies.size() && i < envIt->second.size(); ++i) {
				auto st = strategy_tex.find(State.environment_strategies[i]);
				if (st == strategy_tex.end() || !st->second) continue;
				float tw = 0.f, th = 0.f;
				if (!SDL_GetTextureSize(st->second, &tw, &th) || tw <= 0.f || th <= 0.f) continue;
				SDL_FRect dst = FitRectCentered(envIt->second[i], tw, th);
				SDL_RenderTexture(rdr, st->second, NULL, &dst);
			}
		}

		//选中的环境画一圈白色边框
		if (envIt != ui_place.end() &&
			State.environment_selected >= 0 &&
			State.environment_selected < static_cast<int>(envIt->second.size())) {
			const SDL_FRect& rect = envIt->second[static_cast<size_t>(State.environment_selected)];
			SDL_SetRenderDrawColor(rdr, 255, 255, 255, 255);
			const float thickness = 6.f;
			for (float i = 0.f; i < thickness; i += 1.f) {
				SDL_FRect border{
					rect.x + i,
					rect.y + i,
					rect.w - 2.f * i,
					rect.h - 2.f * i
				};
				SDL_RenderRect(rdr, &border);
			}
		}

		//刷新按钮：还有刷新次数时才绘制
		if (State.environment_refresh_left > 0) {
			auto placeIt = ui_place.find("refresh");
			auto texIt = ui_tex.find("refresh");
			if (placeIt != ui_place.end() && !placeIt->second.empty() &&
				texIt != ui_tex.end()) {
				auto t = texIt->second.find("Allowed");
				if (t != texIt->second.end() && t->second) {
					SDL_RenderTexture(rdr, t->second, NULL, &placeIt->second[0]);
				}
			}
		}

		//确认按钮：未选择 / 已选择两种贴图
		{
			auto placeIt = ui_place.find("confirm");
			auto texIt = ui_tex.find("confirm");
			if (placeIt != ui_place.end() && !placeIt->second.empty() &&
				texIt != ui_tex.end()) {
				const char* name = State.environment_selected >= 0 ? "Selected" : "NotSelected";
				auto t = texIt->second.find(name);
				if (t != texIt->second.end() && t->second) {
					SDL_RenderTexture(rdr, t->second, NULL, &placeIt->second[0]);
				}
			}
		}
	}
}