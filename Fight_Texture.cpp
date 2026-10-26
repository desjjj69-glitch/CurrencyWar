#include"Fight_Texture.h"
#include"Fight_Logic.h"
#include<fstream>
#include"json.hpp"
#include"Logger.h"
#include<filesystem>
#include<algorithm>
using json = nlohmann::json;
using Logger::Log;
namespace Fight {
	void Fight_Texture::Draw(SDL_Renderer*& rdr, TTF_Font*& ft) {//TODO 待绘制内容：
		//TODO 角色大招图标，角色头像，角色技能，敌人图标，目标选择
		//TODO 
		SDL_SetRenderDrawColor(rdr, 0, 0, 0, 255);
		SDL_RenderClear(rdr);

		Draw_Background(rdr);
		Draw_ActionList(rdr);
		Draw_Character(rdr, ft);
		Draw_BackCharacter(rdr, ft);
		Draw_CharacterSelecting(rdr, ft);
		Draw_Enemy(rdr);//TODO 要建立从ui_place到真实位置的双射，因为可能有的地方是nullptr
		//算了不建立也行，nullptr的位置不绘制就行
		Draw_EnemyTargetChoose(rdr);
		Draw_SkillPoint(rdr, ft);
		if (State.AtFight == Fight_Logic::FightGlobalState::Pause) {
			Draw_Pause(rdr, ft);

		}
		
	}
	Fight_Texture::~Fight_Texture() {

		auto KillMap = [](auto& map) {
			for (auto& [k, tex] : map) {
				if (tex) SDL_DestroyTexture(tex);
			}
			map.clear();
			};

		KillMap(actor_tex);
		KillMap(extraactor_tex);
		KillMap(character_tex);
		KillMap(enemy_tex);
		if (energy_text) { SDL_DestroyTexture(energy_text); energy_text = nullptr; }
		if (skillpoint_text) { SDL_DestroyTexture(skillpoint_text); skillpoint_text = nullptr; }
		for (auto& [type, inner] : dynamic_ui_tex) {
			KillMap(inner);
		}
		dynamic_ui_tex.clear();

		// 值类型，clear 只是提前还内存，不 clear 也会随对象析构自动释放
		dynamic_ui_path.clear();
		ui_place.clear();

	}
	void Fight_Texture::Loadin(const std::filesystem::path& gp, SDL_Renderer*& rdr) {
		
			LoadinJson(gp);
			
			LoadinPic(gp, rdr);
			
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

		//TODO 这里可能有严重的内存泄露问题：如果两个敌人是相同的，会重复加载，但由于这个函数只执行一次看不出来
		//接下来导入行动条头像
		//基础行动条
		std::filesystem::path actp=gp / "assets" / "Fight" / "ActorHead";
		for (auto& var : actor_name) {
			auto tmp = IMG_LoadTexture(rdr, (actp / (var + ".png")).string().c_str());
			if (actor_tex[var])SDL_DestroyTexture(actor_tex[var]);
			actor_tex[var] = tmp;
		}
		//额外行动回合
		std::filesystem::path extp = gp / "assets" / "Fight" / "ExtraHead";
		for (auto& var : actor_name) {
			auto tmp = IMG_LoadTexture(rdr, (extp / (var + ".png")).string().c_str());
			if (extraactor_tex[var])SDL_DestroyTexture(extraactor_tex[var]);
			extraactor_tex[var] = tmp;
		}
		//接下来导入角色头像
		std::filesystem::path chp = gp / "assets" / "Fight" / "Character_Head";
		for (auto& var : State.Actor(Fight_Logic::PlaceType::FrontCharacter)) {
			if (!var)continue;
			if (character_tex[var->Name()])SDL_DestroyTexture(character_tex[var->Name()]);
			character_tex[var->Name()] = IMG_LoadTexture(rdr, (chp / (var->Name() + ".png")).string().c_str());
		}
		for (auto& var : State.Actor(Fight_Logic::PlaceType::BackCharacter)) {
			if (!var)continue;
			if (character_tex[var->Name()])SDL_DestroyTexture(character_tex[var->Name()]);
			character_tex[var->Name()] = IMG_LoadTexture(rdr, (chp / (var->Name() + ".png")).string().c_str());
		}
		//导入敌人大图
		std::filesystem::path enep = gp / "assets" / "Enemy";
		for (auto& var : State.Actor(Fight_Logic::PlaceType::Enemy)) {
			if (!var)continue;
			if (enemy_tex[var->Name()])SDL_DestroyTexture(enemy_tex[var->Name()]);
			enemy_tex[var->Name()] = IMG_LoadTexture(rdr, (enep / (var->Name() + ".png")).string().c_str());
		}
		for (auto& var : State.GetEnemyPool()) {
			if (!var)continue;
			if (enemy_tex[var->Name()])SDL_DestroyTexture(enemy_tex[var->Name()]);
			enemy_tex[var->Name()] = IMG_LoadTexture(rdr, (enep / (var->Name() + ".png")).string().c_str());
		}
		//Log("new", __func__);
	}
	SDL_Texture* Fight_Texture::GetUITex(const std::string& uitype, const std::string& uiname) {//是哪个种类的ui,和具体是谁的，比如是背景：翁法罗斯,如果不成功，会默认返回第二个字段为Default的tex
		auto itType = dynamic_ui_tex.find(uitype);
		if (itType == dynamic_ui_tex.end()) return nullptr;

		auto it = itType->second.find(uiname);
		if (it == itType->second.end() || !it->second) {
			it = itType->second.find("Default");
			if (it == itType->second.end() || !it->second)return nullptr;
			return it->second;
		}
		return it->second;
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
		for (size_t i = 0; i < State.ExtraAction.size(); ++i) {
			auto* p = State.FindActorFromExtraAction(i);
			if (!p) break;

			auto key = p->Name();
			if (p->Type() == ActorType::Enemy) {
				key += "_Head";              // ← 这里补上
			}

			SDL_Texture* tex = extraactor_tex[key];
			if (!tex) {
				// 也可以再退一步：用 actor_tex[key] 兜底
				tex = actor_tex[key];
			}
			if (tex) {
				SDL_RenderTexture(rdr, tex, NULL, &exrect);
			}

			exrect.x += exrect.w;
			if (exrect.x > actionlist_place.extraendx) break;
		}
	}
	void Fight_Texture::Draw_Pause(SDL_Renderer*& rdr, TTF_Font*& ft) {
		int w = 0, h = 0;
		

		
		SDL_SetRenderDrawColor(rdr, 128, 128, 128, 128);
		SDL_FRect full{ 0.f, 0.f, 2560, 1600 };
		SDL_RenderFillRect(rdr, &full);

		

	}
	void Fight_Texture::Draw_Enemy(SDL_Renderer*& rdr) {
		for (size_t i = 0;i < ui_place["Enemy"].size();i++) {
			auto& vec = State.Actor(Fight_Logic::PlaceType::Enemy);
			if (i >= vec.size())continue;
			//Log("Draw_Enemyhere", __FUNCSIG__);
			if (vec[i]) {
				auto* p = dynamic_cast<Enemy_Actor*>(vec[i].get());
				if (!p) continue;
				if (p->CurrentHeart <= 0) continue;          // 已死亡不绘制（头像+血条）

				SDL_FRect head = ui_place["Enemy"][i];

				// 头像
				SDL_RenderTexture(rdr, enemy_tex[vec[i]->Name()], NULL, &head);

				// 血条：头像正上方
				SDL_FRect barBg = {
					head.x,
					head.y - 12.f,        // 10 高 + 2 间隙
					head.w,
					10.f
				};

				// 底：深灰
				SDL_SetRenderDrawColor(rdr, 60, 60, 60, 255);
				SDL_RenderFillRect(rdr, &barBg);

				// 前景：按比例
				double ratio = 1.0;
				if (p->MaxHeart > 0)
					ratio = static_cast<double>(p->CurrentHeart)
					/ static_cast<double>(p->MaxHeart);
				ratio = std::clamp(ratio, 0.0, 1.0);

				SDL_FRect barFg = barBg;
				barFg.w = static_cast<float>(barBg.w * ratio);

				if (ratio > 0.5)
					SDL_SetRenderDrawColor(rdr, 80, 220, 80, 255);
				else if (ratio > 0.2)
					SDL_SetRenderDrawColor(rdr, 230, 200, 60, 255);
				else
					SDL_SetRenderDrawColor(rdr, 220, 60, 60, 255);

				SDL_RenderFillRect(rdr, &barFg);
				SDL_SetRenderDrawColor(rdr, 0, 0, 0, 255);
			}
		}
	}
	void Fight_Texture::Draw_EnemyTargetChoose(SDL_Renderer*& rdr) {
		if (State.CurrentState != Fight_Logic::StateType::PlayerSelecting) return;
		if (!State.NowActing || State.NowActing->Type() != ActorType::Character) return;

		auto& mainPlaces = ui_place["EnemyTargetChoose_Main"];
		auto& subPlaces = ui_place["EnemyTargetChoose_Sub"];

		for (auto& var : State.skill_targets.list) {
			//Log("Here1", "Debug");
			if (var.role == TargetRole::Main) {
				//Log("Here1.5", "Debug");
				if (var.index >= mainPlaces.size()) continue;
				//Log("Here2", "Debug");
				//Log("here3", "Debug");
				SDL_Texture* tex = GetUITex("EnemyTargetChoose", "Main");
				if (tex) {
					//Log("Here3", "Debug");
					SDL_RenderTexture(rdr, tex, nullptr, &mainPlaces[var.index]);
				}
			}
			else {
				if (var.index >= subPlaces.size()) continue;

				SDL_Texture* tex = GetUITex("EnemyTargetChoose", "Default");
				if (tex) {
					SDL_RenderTexture(rdr, tex, nullptr, &subPlaces[var.index]);
				}
			}
		}
	}
	void Fight_Texture::Draw_Character(SDL_Renderer*& rdr, TTF_Font*& ft) {
		for (size_t i = 0;i < ui_place["FrontCharacter"].size();i++) {
			auto& vec = State.Actor(Fight_Logic::PlaceType::FrontCharacter);
			if (i >= vec.size())continue;
			if (vec[i]) {
				SDL_RenderTexture(rdr, character_tex[vec[i]->Name()], NULL, &ui_place["FrontCharacter"][i]);//角色头像
				Character_Actor* p=nullptr;
				if (vec[i]->Type() == ActorType::Character) {
					p = static_cast<Character_Actor*>(vec[i].get());
				}
				else {
					continue;
				}
				if (p->Energy >= p->config.basic_attribute.EnergyLimit) {//大招图标
					SDL_RenderTexture(rdr, GetUITex("Ult_Full",p->name), NULL, &ui_place["Ult_Full"][i]);
				}
				else {
					SDL_RenderTexture(rdr, GetUITex("Ult_NotFull", p->name), NULL, &ui_place["Ult_NotFull"][i]);
					//绘制数值
					std::string energyText =
						std::to_string(p->Energy) + "/"
						+ std::to_string(p->config.basic_attribute.EnergyLimit);

					SDL_Color white = { 255, 255, 255, 255 };

					// 先用 0,0 生成，然后按 Rect 尺寸算居中位置
					MySDL::DisplayWord(ft, rdr, energyText,
						0.f, 0.f,
						energy_text, energy_rect,
						white, 1.0f);

					// 居中
					energy_rect.x = ui_place["Ult_Full"][i].x + (ui_place["Ult_Full"][i].w - energy_rect.w) * 0.5f;
					energy_rect.y = ui_place["Ult_Full"][i].y + (ui_place["Ult_Full"][i].h - energy_rect.h) * 0.5f;

					// 真正画到屏幕上
					SDL_RenderTexture(rdr, energy_text, nullptr, &energy_rect);
				}
				SDL_FRect head = ui_place["FrontCharacter"][i];
				SDL_FRect ult = ui_place["Ult_Full"][i];   // Ult_NotFull 位置相同，取哪个都行

				// 绘制血条
				float barLeft = std::min(head.x, ult.x);
				float barRight = std::max(head.x + head.w, ult.x + ult.w);
				float barBottom = std::max(head.y + head.h, ult.y + ult.h);

				SDL_FRect barBg = {
					barLeft,
					barBottom + 2.f,             // 底下留 2px 间隙
					barRight - barLeft,          // 宽度 = 头像+大招的总跨度
					10.f                         // 高度 10
				};

				// 底：深灰
				SDL_SetRenderDrawColor(rdr, 60, 60, 60, 255);
				SDL_RenderFillRect(rdr, &barBg);

				// 前景：按 heart / MaxHeart 比例
				double ratio = 1.0;
				if (p->MaxHeart > 0)
					ratio = static_cast<double>(p->heart) / static_cast<double>(p->MaxHeart);
				ratio = std::clamp(ratio, 0.0, 1.0);

				SDL_FRect barFg = barBg;
				barFg.w = static_cast<float>(barBg.w * ratio);

				if (ratio > 0.5)
					SDL_SetRenderDrawColor(rdr, 80, 220, 80, 255);   // 绿
				else if (ratio > 0.2)
					SDL_SetRenderDrawColor(rdr, 230, 200, 60, 255);  // 黄
				else
					SDL_SetRenderDrawColor(rdr, 220, 60, 60, 255);   // 红

				SDL_RenderFillRect(rdr, &barFg);
				SDL_SetRenderDrawColor(rdr, 0, 0, 0, 255);



			}
		}
		
	}
	void Fight_Texture::Draw_BackCharacter(SDL_Renderer*& rdr, TTF_Font*& ft) {
		for (size_t i = 0;i < ui_place["BackCharacter"].size();i++) {
			auto& vec = State.Actor(Fight_Logic::PlaceType::BackCharacter);
			if (i >= vec.size())continue;
			if (vec[i]) {
				SDL_RenderTexture(rdr, character_tex[vec[i]->Name()], NULL, &ui_place["BackCharacter"][i]);
			}
		}
	}
	void Fight_Texture::Draw_CharacterSelecting(SDL_Renderer*& rdr, TTF_Font*& ft) {//TODO 现在暂时不接受大招
		if (State.CurrentState != Fight_Logic::StateType::PlayerSelecting)return;
		if (State.NowActing->Type() != ActorType::Character)return;
		switch (State.CurrentCharacterState) {
		case Fight_Logic::CharacterState::Normal:
			SDL_RenderTexture(rdr, GetUITex("Normal", State.NowActing->Name()), NULL, &ui_place["Normal"][0]);
			SDL_RenderTexture(rdr, GetUITex("Skill", State.NowActing->Name()), NULL, &ui_place["Skill"][1]);
			break;
		case Fight_Logic::CharacterState::Skill:
			SDL_RenderTexture(rdr, GetUITex("Normal", State.NowActing->Name()), NULL, &ui_place["Normal"][1]);
			SDL_RenderTexture(rdr, GetUITex("Skill", State.NowActing->Name()), NULL, &ui_place["Skill"][0]);
			break;
		}
	}
	void Fight_Texture::Draw_SkillPoint(SDL_Renderer*& rdr, TTF_Font*& ft) {
		auto it = ui_place.find("SkillPoint");
		if (it == ui_place.end() || it->second.empty()) return;
		SDL_Color white = { 255, 255, 255, 255 };
		MySDL::DisplayWord(ft, rdr, std::to_string(State.SkillPoint),
			it->second[0].x, it->second[0].y,
			skillpoint_text, skillpoint_rect,
			white, 1.5f);
		if (skillpoint_text) {
			SDL_RenderTexture(rdr, skillpoint_text, nullptr, &skillpoint_rect);
		}
	}
}