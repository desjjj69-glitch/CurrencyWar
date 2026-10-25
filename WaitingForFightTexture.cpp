#include"WaitingForFightTexture.h"
#include"WaitingForFight_InteractLogic.h"
#include"Mineral.h"
#include"Equipment/BasicEquipment.h"
#include<fstream>
#include<sstream>
#include"Logger.h"
#include"json.hpp"
using Logger::Log;
using json = nlohmann::json;
namespace WaitingForFight {
	void WaitingForFightTexture::Word_Simplepara(SDL_Renderer*& rdr, TTF_Font*& ft, const std::string& text, const std::string& key) {//text,key
		MySDL::DisplayWord(
			ft,
			rdr,
			text,
			word_place[key].x,
			word_place[key].y,
			word_tex[key],
			word_place[key],
			word_color[key],
			word_multiply[key]
		);
	}
	void WaitingForFightTexture::Draw(SDL_Renderer*& rdr,TTF_Font*& ft) {
		SDL_SetRenderDrawColor(rdr, 0, 0, 0, 255);
		SDL_RenderClear(rdr);

		
		Draw_Background(rdr);
		Draw_QuitButton(rdr);
		Draw_Blood(rdr, ft);
		Draw_Stage(rdr, ft);
		Draw_CharacterOnBattleFieldLimit(rdr, ft);
		Draw_ContinueWin(rdr, ft);
		Draw_BuyExp(rdr, ft);
		Draw_ShopButton(rdr, ft);
		Draw_EnemyPreview(rdr, ft);
		Draw_StartFightButton(rdr, ft);
		Draw_FrontBattleField(rdr);
		Draw_BackBattleField(rdr);
		Draw_Bench(rdr);
		Draw_Equipment(rdr);
		Draw_Mineral(rdr);
		if (State.Rmode == Logic::RealTimeMode::Shop) {
			Draw_Shop(rdr,ft);
		}
		if (State.holding) {
			
			if (State.holding->Type() == WaitingForFight::ResourceType::Character) {
				Draw_Sell(rdr, ft);
			}
			Draw_holding(rdr);
		}
		//此处仅绘制一些基本要素，应当最后统一present
		//static bool presig = false;
		//bool npresig=SDL_RenderPresent(rdr);
		//if (presig != npresig) {
		//	presig = npresig;
		//	Log("WaitingForFight_Present", __func__, presig ? Logcode::OK : Logcode::RENDERFAILED);
		//}
	}
	void WaitingForFightTexture::LoadinJson(const std::filesystem::path& gp) {
		std::ifstream configin(gp / "data" / "WaitingForFight.json");
		json js;
		Log("WaitingForFight_Json", __func__, configin.is_open() ? Logcode::OK : Logcode::OPENFAILED);
		try {
			configin >> js;
			auto ParseFRect = [](const std::string& s) -> SDL_FRect {
				SDL_FRect r{ 0.0f, 0.0f, 0.0f, 0.0f };
				std::istringstream iss(s);
				iss >> r.x >> r.y >> r.w >> r.h;
				return r;
				};

			if (js.contains("Picture Name") && js["Picture Name"].is_object()) {
				for (auto& [key, value] : js["Picture Name"].items()) {
					if (value.is_array()) {
						for (auto& v : value) {
							reflect[key].push_back(v.get<std::string>());
						}
					}
					else {
						reflect[key].push_back(value.get<std::string>());
					}
				}
			}
			if (js.contains("Place") && js["Place"].is_object()) {
				for (auto& [key, value] : js["Place"].items()) {
					if (value.is_array()) {
						for (auto& v : value) {
							place[key].push_back(ParseFRect(v.get<std::string>()));
						}
					}
					else {
						place[key].push_back(ParseFRect(value.get<std::string>()));
					}
				}
			}
			if (js.contains("Word") && js["Word"].is_object()) {
				for (auto& [key, value] : js["Word"].items()) {
					std::istringstream vin(value.get<std::string>());
					//Log(value.get<std::string>(), __func__, Logcode::OK);
					int r = 0, g = 0, b = 0, a = 0;
					SDL_FRect rect = {};
					vin >> r >> g >> b >> a>> word_multiply[key]>> rect.x >> rect.y >> rect.w >> rect.h;
					word_color[key] = {
						static_cast<Uint8>(r),
						static_cast<Uint8>(g),
						static_cast<Uint8>(b),
						static_cast<Uint8>(a)
					};
					word_place[key] = rect;
					//Log(std::to_string(rect.x) + " " + std::to_string(rect.y), __func__, Logcode::OK);
				}
			}
		}
		catch (const json::parse_error&) {
			Log("WaitingForFight_Json", __func__, Logcode::PARSEERROR);
		}
		catch (const std::exception& e) {
			Log(e.what(), __func__, Logcode::ERROR);
		}

	}
	void WaitingForFightTexture::LoadinPic(const std::filesystem::path& gp, SDL_Renderer*& rdr) {
		std::filesystem::path picp = gp / "assets" / "WaitingForFight";
		for (auto& [key, vec] : reflect) {
			for (auto& var : vec) {
				try {
					auto tmp = IMG_LoadTexture(rdr, (picp / var).string().c_str());
					if (tmp == nullptr) {
						throw std::runtime_error(
							std::string("IMG_LoadTexture failed: ") + SDL_GetError()
						);
					}
					Log(key + "+" + var, __func__, Logcode::OK);
					tex[key].push_back(tmp);
					picsize[key].push_back({ 0.0,0.0,static_cast<float>(tmp->w),static_cast<float>(tmp->h) });
				}
				catch (const std::exception & e) {
					Log(e.what(), __func__, Logcode::OPENFAILED);
				}
				
			}

		}
	}
	void WaitingForFightTexture::Loadin(const std::filesystem::path& gp, SDL_Renderer*& rdr) {
		if (!Loaded) {
			LoadinJson(gp);

			Loaded = true;
		}
		if (!texLoaded) {
			LoadinPic(gp, rdr);
			texLoaded = true;
		}
		character_tex.Loadin(gp, rdr);
		
		enemy_tex.Loadin(gp, rdr);
		
		equipment_tex.Loadin(gp, rdr);

		mineral_tex.Loadin(gp, rdr);
		
	}
	WaitingForFightTexture::~WaitingForFightTexture() {
		for (auto it = tex.begin();it != tex.end();++it) {
			for (auto jt = it->second.begin();jt != it->second.end();++jt) {
				SDL_DestroyTexture(*jt);
			}
			it->second.clear();
		}
		tex.clear();
		texLoaded = false;
	}
	void WaitingForFightTexture::Draw_Background(SDL_Renderer*& rdr) {
		static bool sig = false;
		bool nsig = SDL_RenderTexture(rdr, tex["background"][0], NULL, &place["background"][0]);
		if (sig != nsig) {
			sig = nsig;
			Log("WaitingForFight_background", __func__, sig ? Logcode::OK : Logcode::RENDERFAILED);
		}

	}
	void WaitingForFightTexture::Draw_QuitButton(SDL_Renderer*& rdr) {
		static bool sig = false;
		static SDL_FRect buttonrect = 
		{ 
			place["Button_Exit"][0].x,
			place["Button_Exit"][0].y,
			picsize["Button_Exit"][0].w,
			picsize["Button_Exit"][0].h 
		};
		bool nsig = SDL_RenderTexture(rdr, tex["Button_Exit"][0], NULL, &buttonrect);
		if (sig != nsig) {
			sig = nsig;
			Log("QuitButton", __func__, sig ? Logcode::OK : Logcode::RENDERFAILED);
		}
	}
	void WaitingForFightTexture::Draw_Blood(SDL_Renderer*& rdr, TTF_Font*& ft) {
		static int old = -1;
		SDL_RenderTexture(rdr, tex["Blood_TotalNum"][0], NULL, &place["Blood_TotalNum"][0]);
		if (State.Blood != old) {
			old = State.Blood;
			MySDL::DisplayWord(
				ft, rdr, 
				std::to_string(old), 
				word_place["Blood"].x, 
				word_place["Blood"].y, 
				word_tex["Blood"],
				word_place["Blood"], 
				word_color["Blood"], 
				word_multiply["Blood"]
			);
		}
		SDL_RenderTexture(rdr, word_tex["Blood"], NULL, &word_place["Blood"]);
		//Log(std::to_string(word_place["Blood"].x)+" "+ std::to_string(word_place["Blood"].h), __func__, word_tex["Blood"] == nullptr ? Logcode::ERROR : Logcode::OK);
	}
	void WaitingForFightTexture::Draw_Stage(SDL_Renderer*& rdr, TTF_Font*& ft) {
		//备战阶段
		static int olddimension = -1;
		static int oldstage = -1;
		SDL_RenderTexture(rdr, tex["Show_Stage"][0], NULL, &place["Show_Stage"][0]);//绘制背景
		if (static_cast<int>(State.dimension_index) != olddimension || static_cast<int>(State.stage_index) != oldstage) {
			olddimension = static_cast<int>(State.dimension_index);
			oldstage = static_cast<int>(State.stage_index);
			MySDL::DisplayWord(
				ft,
				rdr,
				std::to_string(olddimension+1) + "-" + std::to_string(oldstage+1),
				word_place["ShowStage"].x, word_place["ShowStage"].y,
				word_tex["ShowStage"],
				word_place["ShowStage"],
				word_color["ShowStage"],
				word_multiply["ShowStage"]
			);
		}
		SDL_RenderTexture(rdr, word_tex["ShowStage"], NULL, &word_place["ShowStage"]);

		//战斗节点背景
		SDL_RenderTexture(rdr, tex["Show_StageNodes_Background"][0], NULL, &place["Show_StageNodes_Background"][0]);
	}
	void WaitingForFightTexture::Draw_CharacterOnBattleFieldLimit(SDL_Renderer*& rdr, TTF_Font*& ft) {
		static SDL_FRect buttonrect =
		{
			place["CharacterOnBattleFieldLimit"][0].x,
			place["CharacterOnBattleFieldLimit"][0].y,
			picsize["CharacterOnBattleFieldLimit"][0].w,
			picsize["CharacterOnBattleFieldLimit"][0].h
		};
		
		SDL_RenderTexture(rdr, tex["CharacterOnBattleFieldLimit"][0], NULL, &buttonrect);
		//绘制一个数字/数字
		static int oldlimit = -1;
		static int oldnum = -1;
	}
	void WaitingForFightTexture::Draw_ContinueWin(SDL_Renderer*& rdr, TTF_Font*& ft) {
		static int old = -1;
		SDL_RenderTexture(rdr, tex["ContinueWin"][0], NULL, &place["ContinueWin"][0]);
		if (old != State.ContinueWinTimes) {
			old = State.ContinueWinTimes;
			MySDL::DisplayWord(
				ft,
				rdr,
				std::to_string(old),
				word_place["ContinueWinTimes"].x,
				word_place["ContinueWinTimes"].y,
				word_tex["ContinueWinTimes"],
				word_place["ContinueWinTimes"],
				word_color["ContinueWinTimes"],
				word_multiply["ContinueWinTimes"]
			);
		}
		SDL_RenderTexture(rdr, word_tex["ContinueWinTimes"], NULL, &word_place["ContinueWinTimes"]);
	}
	void WaitingForFightTexture::Draw_BuyExp(SDL_Renderer*& rdr, TTF_Font*& ft) {
		static int oldshoplevel = -1;
		static int oldexpnum = -1;
		int expsum = 0;
		if (State.shop.MaxShopLevel()) {
			MySDL::DisplayTexture_DefaultSize(rdr, tex["MaxShopPoolLevel"][0], place["MaxShopPoolLevel"][0]);
			return;
		}
		SDL_RenderTexture(rdr, tex["BuyExp"][0], NULL, &place["BuyExp"][0]);
		if (oldshoplevel != State.shop.ShopLevel || oldexpnum != State.shop.TotalExp) {
			oldshoplevel = State.shop.ShopLevel;
			oldexpnum = State.shop.TotalExp;
			MySDL::DisplayWord(
				ft,
				rdr,
				std::to_string(State.shop.ShopLevel),
				word_place["ShopLevel"].x,
				word_place["ShopLevel"].y,
				word_tex["ShopLevel"],
				word_place["ShopLevel"],
				word_color["ShopLevel"],
				word_multiply["ShopLevel"]
			);
			expsum = State.shop.TotalExp-State.shop.CalculateExpSum();
			Word_Simplepara(rdr, ft, std::to_string(expsum) + "/" + std::to_string(State.shop.explevelup[State.shop.ShopLevel + 1]), "ExpNum");
		}
		SDL_RenderTexture(rdr, word_tex["ShopLevel"], NULL, &word_place["ShopLevel"]);
		SDL_FRect rect = {
			word_place["ExpNum"].x - static_cast<float>(word_tex["ExpNum"]->w) / 2,
			word_place["ExpNum"].y,
			word_place["ExpNum"].w,
			word_place["ExpNum"].h
		};
		SDL_RenderTexture(rdr, word_tex["ExpNum"], NULL, &rect);
	}
	void WaitingForFightTexture::Draw_ShopButton(SDL_Renderer*& rdr, TTF_Font*& ft) {
		static Logic::RealTimeMode oldmode = Logic::RealTimeMode::Normal;
		static int oldinterest = -1;
		static int oldmoney = -1;
		
		if (oldinterest != (State.MoneyNum / 10 >= State.MaxInterest ? State.MaxInterest : State.MoneyNum / 10)) {
			oldinterest = (State.MoneyNum / 10 >= State.MaxInterest ? State.MaxInterest : State.MoneyNum / 10);
			
			Word_Simplepara(rdr, ft, std::to_string(oldinterest), "Interest");
			//Log(std::to_string(word_multiply["Interest"]), __func__, Logcode::OK);
		}
		
		auto displaymoney = [&](const std::string& key) {
			if (oldmoney != State.MoneyNum||
				oldmode!=State.Rmode
				) {
				oldmoney = State.MoneyNum;
				oldmode = State.Rmode;
				Word_Simplepara(rdr, ft, std::to_string(oldmoney), key);
			}
			SDL_RenderTexture(rdr, word_tex[key], NULL, &word_place[key]);
			};
		switch (State.Rmode) {
		
		case Logic::RealTimeMode::Shop:
			SDL_RenderTexture(rdr, tex["Shop_CloseShopButton"][0], NULL, &place["Shop_CloseShopButton"][0]);
			displaymoney("Money_Shop");
			break;
		case Logic::RealTimeMode::Normal:
		default:
			SDL_RenderTexture(rdr, tex["OpenShop"][0], NULL, &place["OpenShop"][0]);
			displaymoney("Money_Normal");
			break;
		}
		SDL_RenderTexture(rdr, word_tex["Interest"], NULL, &word_place["Interest"]);
	}
	void WaitingForFightTexture::Draw_StartFightButton(SDL_Renderer*& rdr, TTF_Font*& ft) {
		MySDL::DisplayTexture_DefaultSize(rdr, tex["StartFight"][0], place["StartFight"][0]);
	}
	void WaitingForFightTexture::Draw_Shop(SDL_Renderer*& rdr,TTF_Font*& ft) {
		//Log("Here", __func__);
		SDL_RenderTexture(rdr, tex["Shop_Background"][0], NULL, &place["Shop_Background"][0]);
		SDL_RenderTexture(rdr, tex["Shop_DcardButton"][0], NULL, &place["Shop_DcardButton"][0]);
		static int olddcardm = -1;
		if (olddcardm != State.shop.Dcardmoney) {
			Word_Simplepara(rdr, ft, std::to_string(State.shop.Dcardmoney), "DcardMoney");
		}
		SDL_RenderTexture(rdr, word_tex["DcardMoney"], NULL, &word_place["DcardMoney"]);
		for (int i = 0;i < State.shop.RealShop.size();i++) {
			if (WaitingForFight::State.shop.RealShop[i]) {
				auto* c = dynamic_cast<CharacterObject*>(State.shop.RealShop[i].get());
				if (!c) {
					return;
				}
				SDL_RenderTexture(
					rdr,
					character_tex.GetCharacterTexture(c->name),
					NULL,
					&State.ObjectPlace(Logic::OriginPlaceType::ShopCharacter)[i]
				);
			}
		}
	}
	void WaitingForFightTexture::Draw_Bench(SDL_Renderer*& rdr) {
		SDL_RenderTexture(rdr, tex["CharacterOnBench_Background"][0], NULL, &place["CharacterOnBench_Background"][0]);
		//Log(std::to_string(State.Zone(Logic::OriginPlaceType::Bench).size()), __func__);
		for (int i = 0;i < State.Zone(Logic::OriginPlaceType::Bench).size();i++) {
			if (State.Zone(Logic::OriginPlaceType::Bench)[i]) {
				Draw_Character(rdr, State.Zone(Logic::OriginPlaceType::Bench)[i], State.ObjectPlace(Logic::OriginPlaceType::Bench)[i]);
			}
		}
	}
	void WaitingForFightTexture::Draw_Mineral(SDL_Renderer*& rdr) {
		auto& zone = State.Zone(Logic::OriginPlaceType::Mineral);
		auto& places = State.ObjectPlace(Logic::OriginPlaceType::Mineral);
		const size_t count = zone.size() < places.size() ? zone.size() : places.size();
		for (size_t i = 0; i < count; ++i) {
			if (!zone[i]) continue;
			auto* m = dynamic_cast<Mineral*>(zone[i].get());
			if (!m) continue;
			SDL_Texture* t = mineral_tex.GetMineralTexture(m->Quality());
			if (!t) continue;
			SDL_RenderTexture(rdr, t, NULL, &places[i]);
		}
	}
	void WaitingForFightTexture::Draw_Equipment(SDL_Renderer*& rdr) {
		auto DrawZone = [&](Logic::OriginPlaceType type) {
			auto& zone = State.Zone(type);
			auto& places = State.ObjectPlace(type);
			const size_t count = zone.size() < places.size() ? zone.size() : places.size();
			for (size_t i = 0; i < count; ++i) {
				if (!zone[i]) continue;
				auto* e = dynamic_cast<BasicEquipment*>(zone[i].get());
				if (!e) continue;
				SDL_Texture* t = equipment_tex.GetEquipmentTexture(e->GetName());
				if (!t) continue;
				SDL_RenderTexture(rdr, t, NULL, &places[i]);
			}
			};
		DrawZone(Logic::OriginPlaceType::ConsumableEquipment);
		DrawZone(Logic::OriginPlaceType::Equipment);
	}
	void WaitingForFightTexture::Draw_FrontBattleField(SDL_Renderer*& rdr) {
		SDL_RenderTexture(rdr, tex["Character_FrontBattleField_Background"][0], NULL, &place["Character_FrontBattleField_Background"][0]);
		for (int i = 0;i < State.Zone(Logic::OriginPlaceType::FrontBattleField).size();i++) {
			if (State.Zone(Logic::OriginPlaceType::FrontBattleField)[i]) {
				Draw_Character(rdr, State.Zone(Logic::OriginPlaceType::FrontBattleField)[i], State.ObjectPlace(Logic::OriginPlaceType::FrontBattleField)[i]);
			}
		}
	}
	void WaitingForFightTexture::Draw_BackBattleField(SDL_Renderer*& rdr) {
		SDL_RenderTexture(rdr, tex["Character_BackBattleField_Background_6"][0], NULL, &place["Character_BackBattleField_Background_6"][0]);
		for (int i = 0;i < State.Zone(Logic::OriginPlaceType::BackBattleField).size();i++) {
			if (State.Zone(Logic::OriginPlaceType::BackBattleField)[i]) {
				Draw_Character(rdr, State.Zone(Logic::OriginPlaceType::BackBattleField)[i], State.ObjectPlace(Logic::OriginPlaceType::BackBattleField)[i]);
				
			}
		}
	}
	void WaitingForFightTexture::Draw_holding(SDL_Renderer*& rdr) {
		if (!State.holding)return;
		SDL_FRect rect = {
			State.MotionX - State.Offset.x,
			State.MotionY - State.Offset.y,
			State.OriginPlace.w,
			State.OriginPlace.h
		};
		switch (State.holding->Type()) {
		case ResourceType::Character: 
		{
			Draw_Character(rdr, State.holding, rect);
		}
		break;
				
		}
	}
	void WaitingForFightTexture::Draw_Sell(SDL_Renderer*& rdr, TTF_Font*& ft) {
		SDL_RenderTexture(rdr, tex["SellCharacter_Left"][0], NULL, &place["SellCharacter_Left"][0]);
		SDL_RenderTexture(rdr, tex["SellCharacter_Right"][0], NULL, &place["SellCharacter_Right"][0]);
		static int oldworth = -1;
		auto displayworth = [&](const std::string key) {
			Word_Simplepara(rdr, ft, std::to_string(oldworth), key);
			
			};
		if (oldworth != State.shop.calculateworth(State.holding)) {
			oldworth = State.shop.calculateworth(State.holding);
			displayworth("SellCharacter_Left");
			displayworth("SellCharacter_Right");
		}
		//Log(std::to_string(State.MotionX), __func__, Logcode::ERROR);
		SDL_RenderTexture(rdr, word_tex["SellCharacter_Left"], NULL, &word_place["SellCharacter_Left"]);
		SDL_RenderTexture(rdr, word_tex["SellCharacter_Right"], NULL, &word_place["SellCharacter_Right"]);
	}
	void WaitingForFightTexture::Draw_Character(SDL_Renderer*& rdr, std::unique_ptr<BasicObject>& ch,SDL_FRect& rect,int signal) {
		auto* c = dynamic_cast<CharacterObject*>(ch.get());
		if (!c)return;
		SDL_RenderTexture(rdr, character_tex.GetCharacterTexture(c->name), NULL, &rect);
		if (signal == 0) {//不是商店发出的绘制指令
			auto& startex = tex["GridFightComStarRank"][static_cast<size_t>(c->game_attribute.stars)];
			SDL_FRect starrect = {
				rect.x+rect.w/2-static_cast<float>(startex->w)/2,
				rect.y+rect.h- static_cast<float>(startex->h)/2,
				static_cast<float>(startex->w),
				static_cast<float>(startex->h)
			};
			SDL_RenderTexture(rdr,startex , NULL, &starrect);
		}
	}
	void WaitingForFightTexture::Draw_EnemyPreview(SDL_Renderer*& rdr, TTF_Font*& ft) {
		SDL_Texture* t = enemy_tex.GetEnemyTexture(State.Enemys[0].name + "_Head");
		if (!t) {
			t = enemy_tex.GetEnemyTexture(State.Boss[State.dimension_index].name + "_Head");
		}
		SDL_RenderTexture(rdr, t, NULL, &word_place["EnemyPreview"]);
	}
	void WaitingForFightTexture::Draw_GameStartPreview(SDL_Renderer*& rdr, TTF_Font*& ft) {
		for (size_t i = 0;i < State.Boss.size();i++) {
			if (i >= place["GameStartPreview"].size())break;
			SDL_RenderTexture(
				rdr,
				enemy_tex.GetEnemyTexture(State.Boss[i].name),
				NULL,
				&place["GameStartPreview"][i]
			);
		}
	}
}
