#include"Fight_Texture.h"
#include"Fight_Logic.h"
#include<fstream>
#include"json.hpp"
#include"Logger.h"
#include<filesystem>
using json = nlohmann::json;
using Logger::Log;
namespace Fight {
	void Fight_Texture::Draw(SDL_Renderer*& rdr, TTF_Font*& ft) {
		SDL_SetRenderDrawColor(rdr, 0, 0, 0, 255);
		SDL_RenderClear(rdr);

		Draw_Background(rdr);
		Draw_ActionList(rdr);
		if (State.AtFight == Fight_Logic::FightGlobalState::Pause) {
			Draw_Pause(rdr, ft);

		}
		
	}
	Fight_Texture::~Fight_Texture() {
		if (!uiloaded) return;

		for (auto& [type, map] : dynamic_ui_tex) {
			for (auto& [picType, tex] : map) {
				if (tex) SDL_DestroyTexture(tex);
			}
			map.clear();
		}
		dynamic_ui_tex.clear();
		dynamic_ui_path.clear();
		ui_place.clear();

		uiloaded = false;
		jsloaded = false;
	}
	void Fight_Texture::Loadin(const std::filesystem::path& gp, SDL_Renderer*& rdr) {
		if (!jsloaded) {
			LoadinJson(gp);
			jsloaded = true;
		}
		if (!uiloaded) {
			LoadinPic(gp, rdr);
			uiloaded = true;
		}
	}
	void Fight_Texture::ReadActionListPosition(const json& js, ActionListTexture &a) {
		a.startx = js.value("startx", 0.0f);
		a.starty = js.value("starty", 0.0f);
		a.startw = js.value("startw", 0.0f);
		a.starth = js.value("starth", 0.0f);//起始单元格


		a.endy = js.value("endy", 0.0f);//最下方的y

		a.extrax = js.value("extrax", 0.0f);
		a.extray = js.value("extray", 0.0f);
		a.extraw = js.value("extraw", 0.0f);
		a.extrah = js.value("extrah", 0.0f);//额外回合

		a.extraendx = js.value("extraendx", 0.0f);//额外回合的最右方x
	}
	void Fight_Texture::LoadinJson(const std::filesystem::path& gp) {
		std::ifstream uiin(gp / "data" / "Fight_Texture.json");
		json jsui;
		Log("", __FUNCSIG__, uiin.is_open() ? Logcode::OK : Logcode::OPENFAILED);
		auto ParseFRect = [](const std::string& s) -> SDL_FRect {
			SDL_FRect r{ 0.0f, 0.0f, 0.0f, 0.0f };
			std::istringstream iss(s);
			iss >> r.x >> r.y >> r.w >> r.h;
			return r;
			};
		try {
			uiin >> jsui;
			for (const auto& item : jsui.at("ui")) {
				if (!item.contains("type")) continue;
				std::string type = item.at("type").get<std::string>();
				if (!item.contains(type)) continue;

				for (const auto& var : item.at(type)) {
					
					ui_place[type].push_back(ParseFRect(var.value("place", "0 0 0 0")));
				}
			}
			if (jsui.contains("dynamic_ui") && jsui["dynamic_ui"].is_object()) {
				for (auto& [type, map] : jsui["dynamic_ui"].items()) {
					if (!map.is_object()) continue;
					for (auto& [picType, path] : map.items()) {
						dynamic_ui_path[type][picType] = path.get<std::string>();
					}
				}
			}
			if (jsui.contains("actionlist")) {
				auto& arr = jsui.at("actionlist");
				if(!arr.empty())
				ReadActionListPosition(arr[0], actionlist_place);
			}
			if (jsui.contains("actors")) {
				auto& arr = jsui.at("actors");
				actor_name = arr.get<std::vector<std::string>>();
			}
		}
		catch (const json::parse_error&e ) {
			Log(e.what(), __func__, Logcode::PARSEERROR);
		}
		catch (const std::exception& e) {
			Log(e.what(), __func__, Logcode::ERROR);
		}
	}
	void Fight_Texture::LoadinPic(const std::filesystem::path& gp, SDL_Renderer*& rdr) {
		std::filesystem::path uip = gp /"assets"/ "Fight" / "UI_Tex";//导入ui图片
		for (auto& [type, map] : dynamic_ui_path) {
			for (auto& [picType, path] : map) {
				std::filesystem::path full = uip / path;
				try {
					SDL_Texture* tmp = IMG_LoadTexture(rdr, full.string().c_str());
					if (!tmp) {
						throw std::runtime_error(
							std::string("IMG_LoadTexture failed: ") + SDL_GetError()
						);
					}
					Log(type + "+" + picType, __func__, Logcode::OK);
					dynamic_ui_tex[type][picType] = tmp;
				}
				catch (const std::exception& e) {
					Log(e.what(), __func__, Logcode::ERROR);
				}
			}
		}
		//接下来导入行动条头像
		std::filesystem::path actp=gp / "assets" / "Fight" / "ActorHead";
		for (auto& var : actor_name) {
			auto tmp = IMG_LoadTexture(rdr, (actp / (var + ".png")).string().c_str());
			actor_tex[var] = tmp;
		}
	}
	
	void Fight_Texture::Draw_Background(SDL_Renderer*& rdr) {
		auto itType = dynamic_ui_tex.find("Background");
		if (itType == dynamic_ui_tex.end()) return;

		auto it = itType->second.find("Amphoreus");
		if (it == itType->second.end() || !it->second) return;

		SDL_RenderTexture(rdr, it->second, nullptr, &ui_place["Background"][0]);
	}
	void Fight_Texture::Draw_ActionList(SDL_Renderer*& rdr) {
		SDL_FRect nrect = {
			actionlist_place.startx,
			actionlist_place.starty,
			actionlist_place.startw,
			actionlist_place.starth
		};
		if (State.NowActing) {
			SDL_FRect nowrect = {
				nrect.x,
				nrect.y - 0.2f * nrect.h,
				nrect.w * 1.2f,
				nrect.h * 1.2f
			};
			

			auto key = State.NowActing->Name();
			if (State.NowActing->Type() == ActorType::Enemy) {
				key += "_Head";
			}

			SDL_RenderTexture(rdr, actor_tex[key], NULL, &nowrect);
		}
		for (size_t i = 0;i < State.ActionList.size();i++) {
			nrect.y += nrect.h;
			if (nrect.y > actionlist_place.endy)break;//超出了就停止绘制
			auto* p = State.FindActorFromActionList(i);
			if (!p)break;

			auto key = p->Name();
			if (p->Type() == ActorType::Enemy) {
				key += "_Head";
			}
			
			SDL_RenderTexture(rdr, actor_tex[key], NULL, &nrect);
			
			
		}
		SDL_FRect exrect = {
			actionlist_place.extrax,
			actionlist_place.extray,
			actionlist_place.extraw,
			actionlist_place.extrah
		};
		for (size_t i = 0;i < State.ExtraAction.size();i++) {
			auto* p = State.FindActorFromExtraAction(i);
			if (!p)break;
			SDL_RenderTexture(rdr, extraactor_tex[p->Name()], NULL, &exrect);
			exrect.x += exrect.w;
			if (exrect.x > actionlist_place.extraendx)break;
		}
	}
	void Fight_Texture::Draw_Pause(SDL_Renderer*& rdr, TTF_Font*& ft) {
		int w = 0, h = 0;
		

		
		SDL_SetRenderDrawColor(rdr, 128, 128, 128, 128);
		SDL_FRect full{ 0.f, 0.f, 2560, 1600 };
		SDL_RenderFillRect(rdr, &full);

		

	}
}