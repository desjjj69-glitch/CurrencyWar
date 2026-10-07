#include"WaitingForFight_InteractLogic.h"
#include"BasicGameResource.h"
#include"json.hpp"
#include"Logger.h"
#include"StageData.h"
#include"GetRandomNum.h"
#include<fstream>
#include<string>
#include<vector>
#include<unordered_map>
#include<cmath>
#include<cstdlib>
using json = nlohmann::json;
using Logger::Log;
namespace WaitingForFight {
	Logic State;
	bool Logic::PlaceInside(float x, float y, const SDL_FRect& p) {
		return x >= p.x && x <= p.x + p.w && y >= p.y && y <= p.y + p.h;
	}
	void Logic::LoadinJson(const std::filesystem::path& gp) {
		//Log("Here", __FUNCSIG__,  Logcode::OPENFAILED);
		Object::Enemy_LoadinJson(gp, Boss);
		Object::Character_LoadinJson(gp);
		shop.LoadinJson(gp);//先把自成一块的资源读取了

		auto ParseFRect = [](const std::string& s) -> SDL_FRect {
			SDL_FRect r{ 0.0f, 0.0f, 0.0f, 0.0f };
			std::istringstream iss(s);
			iss >> r.x >> r.y >> r.w >> r.h;
			return r;
			};
		std::ifstream configin(gp / "config" / "GameResource.json");//把基本资源读入
		Log("WaitingForFightLogic_Json", __FUNCSIG__, configin.is_open() ? Logcode::OK : Logcode::OPENFAILED);
		if (!configin.is_open())return;
		json js;
		try {
			configin >> js;
			if (js.contains("stage") && js["stage"].is_array())
				stage = js.at("stage").get<std::vector<std::vector<std::string>>>();
			Blood = js.value("Blood", Blood);
			//Log(std::to_string(Blood), __func__);
			MoneyNum = js.value("MoneyNum", MoneyNum);

			//Log(std::to_string(MoneyNum), __func__);
			MaxInterest = js.value("MaxInterest", MaxInterest);
		}
		catch (const json::parse_error& e) {
			Log(e.what(), __FUNCSIG__, Logcode::PARSEERROR);
		}
		catch (const std::exception& e) {
			Log(e.what(), __FUNCSIG__, Logcode::ERROR);
		}
		//读入ui的位置资源，和渲染的位置资源分离，单独留一个副本（渲染判定和位置判定未必相同，有宽松度）
		std::ifstream uiin(gp / "data" / "WaitingForFight.json");
		Log("WaitingForFightLogic_Json", __FUNCSIG__, uiin.is_open() ? Logcode::OK : Logcode::OPENFAILED);
		json jsui;
		try {

			uiin >> jsui;
			Log("Here", __func__);
			if (jsui.contains("Place") && jsui["Place"].is_object()) {
				for (auto& [key, value] : jsui["Place"].items()) {
					if (!value.is_array() || value.empty() || !value[0].is_string()) {
						Log("Place." + key, __func__, Logcode::OK);
						continue;
					}

					buttonplace[key] = ParseFRect(value[0].get<std::string>());
					Log(key + std::to_string(buttonplace[key].x), __func__, Logcode::OK);
				}
			}
		}
		catch (const json::parse_error& e) {
			Log(e.what(), __FUNCSIG__, Logcode::PARSEERROR);
		}
		catch (const std::exception& e) {
			Log(e.what(), __FUNCSIG__, Logcode::ERROR);
		}

		std::ifstream objpin(gp / "data" / "WaitingForFight_ObjectPlace.json");
		json objpjs;
		try {

			objpin >> objpjs;

		}
		catch (const json::parse_error& e) {
			Log(e.what(), __FUNCSIG__, Logcode::PARSEERROR);
		}
		catch (const std::exception& e) {
			Log(e.what(), __FUNCSIG__, Logcode::ERROR);
		}
		if (objpjs.contains("Place") && objpjs["Place"].is_object()){
			static const std::unordered_map<std::string, OriginPlaceType> seqTypeMap = {
		{ "ShopCharacter",   OriginPlaceType::ShopCharacter   },
		{ "Bench",           OriginPlaceType::Bench           },
		{ "FrontBattleField",OriginPlaceType::FrontBattleField},
		{ "BackBattleField", OriginPlaceType::BackBattleField },
			};
			for (auto& [seqName, seqArr] : objpjs["Place"].items()) {

				auto it = seqTypeMap.find(seqName);
				if (it == seqTypeMap.end()) {
					// 未知序列，跳过
					continue;
				}

				if (!seqArr.is_array()) {
					Log("Not Array: " + seqName, __func__, Logcode::PARSEERROR);
					continue;
				}

				auto& vec = ObjectPlace(it->second);
				vec.clear();  // 防止重复读取时累积

				for (size_t i = 0; i < seqArr.size(); ++i) {
					if (!seqArr[i].is_string()) {
						Log("Not String: " + seqName + "[" + std::to_string(i) + "]",
							__func__, Logcode::PARSEERROR);
						continue;
					}

					vec.push_back(ParseFRect(seqArr[i].get<std::string>()));
				}
			}
		}
	}
	Logic::Logic() {
		Zone(OriginPlaceType::FrontBattleField).resize(4);
		Zone(OriginPlaceType::Bench).resize(9);
		Zone(OriginPlaceType::BackBattleField).resize(6);
	}
	
	void Logic::HandleEvent(const SDL_Event& event) {
		switch (event.type) {
		case SDL_EVENT_QUIT:
			exit(0);
			break;
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
			if (event.button.button == SDL_BUTTON_LEFT) {
				Mmode = MouseMode::Pressed;
				PressStartTime = SDL_GetTicks();
				PressStartX = event.button.x;
				PressStartY = event.button.y;
			}
			break;
		case SDL_EVENT_MOUSE_BUTTON_UP:
			if (Mmode == MouseMode::Pressed) {
				// 没到长按、没拖动 → 算点击
				Click();
			}
			else if (Mmode == MouseMode::Dragging) {
				// 放下
				if (!PutDown(event.button.x,event.button.y)) {
					PutBack();
				}
			}
			else if (Mmode == MouseMode::LongPressed) {
				//LongPress();
			}
			Mmode = MouseMode::Idle;
			break;
		case SDL_EVENT_MOUSE_MOTION:
			if (Mmode == MouseMode::LongPressed
				|| (Mmode == MouseMode::Pressed && DragDisTest(event.motion))
				) {
				// 已经长按了再移动 → 直接进 Dragging
				// 或者：移动距离超过阈值 → 进 Dragging
				if(PickUp())
				Mmode = MouseMode::Dragging;
				
				
			}
			break;
		case SDL_EVENT_WINDOW_FOCUS_LOST:
			if (holding) PutBack();
			Mmode = MouseMode::Idle;
			break;
		}
	}
	void Logic::EventLogic(SDL_Event& event) {
		Uint64 now = 0;
		while (SDL_PollEvent(&event)) {
			SDL_ConvertEventToRenderCoordinates(BasicGameRes.GameRenderer, &event);
			HandleEvent(event);
			if (event.type == SDL_EVENT_MOUSE_MOTION) {
				MotionX = event.motion.x;
				MotionY = event.motion.y;
			}
			
		}
		if (Mmode == MouseMode::Pressed) {
			now = SDL_GetTicks();
			if (now - PressStartTime >= LONG_PRESS_MS) {
				Mmode = MouseMode::LongPressed;
				LongPress();
			}
		}
	}
	bool Logic::DragDisTest(const SDL_MouseMotionEvent& motion) {
		return
			fabs(PressStartX - motion.x) * fabs(PressStartX - motion.x)
			+ fabs(PressStartY - motion.y) * fabs(PressStartY - motion.y)
			>= CAN_DRAG_DISTANCE* CAN_DRAG_DISTANCE;
	}
	
	int Logic::PickUp() {
		for (int i = 0;i < kPlaceCount;i++) {
			for (int j = 0;static_cast<size_t>(j) < objectplace_[i].size();j++) {
				if (i == static_cast<int>(OriginPlaceType::ShopCharacter)) continue; // 商店不参与拾取，必须购买
				if (zone_[i].size() != objectplace_[i].size()) continue;//大小不一的都给我挡出去
				if (PlaceInside(PressStartX, PressStartY, objectplace_[i][j])) {
					if (zone_[i][j] == nullptr) {
						return 0;
					}
					holdoriginplace = static_cast<OriginPlaceType>(i);
					OriginIndex = j;
					OriginPlace = objectplace_[i][j];
					Offset = { PressStartX- objectplace_[i][j].x,PressStartY- objectplace_[i][j].y,0,0 };
					holding = std::move(zone_[i][j]);
					return 1;
				}
			}
		}
		return 0;
	}
	int Logic::PutDown(float x, float y) {
		if (!holding) {
			return 0;
		}
		auto CheckIfBattleFieldDulplicate = [&](OriginPlaceType t,int tryi,const std::string& name="") {
			auto& z = Zone(t);
			auto* h = static_cast<CharacterObject*>(holding.get());
			for (int j = 0;j < z.size();j++) {

				if (!z[static_cast<size_t>(j)]||j==tryi) {
					continue;
				}
				auto* c = static_cast<CharacterObject*>(z[j].get());
				if (name != "") {
					if (c->name == name) {
						return 1;
					}
				}
				else
				if (c->name == h->name) {
					return 1;
				}
			}
			
			return 0;
			};
		switch (holding->Type()) {
		case ResourceType::Character:
			//只有角色有“出售”这一逻辑
			if (PlaceInside(x, y, buttonplace["SellCharacter_Left"]) ||
				PlaceInside(x, y, buttonplace["SellCharacter_Right"])) {
				
				shop.Sell(holding, MoneyNum);
				return 0;
			}
			for (int i = 0;i < ObjectPlace(OriginPlaceType::FrontBattleField).size();i++) {//前台检测
				
				if (PlaceInside(x, y, ObjectPlace(OriginPlaceType::FrontBattleField)[i])) {
					if (Zone(OriginPlaceType::FrontBattleField)[i] == nullptr) {//放置的逻辑
						if (CountResouceNum(OriginPlaceType::FrontBattleField)
							+ CountResouceNum(OriginPlaceType::BackBattleField)+1 > CharacterOnBattleFieldLimit) {
							return 0;
						}//超员了
						if (CheckIfBattleFieldDulplicate(OriginPlaceType::FrontBattleField,i)||
							CheckIfBattleFieldDulplicate(OriginPlaceType::BackBattleField, -1)
							) {
							return 0;
						}//有重复角色在出战席
						Zone(OriginPlaceType::FrontBattleField)[i] = std::move(holding);
						return 1;//成功放置
					}
					else {//交换的逻辑
						if (CheckIfBattleFieldDulplicate(OriginPlaceType::FrontBattleField, i) ||
							CheckIfBattleFieldDulplicate(OriginPlaceType::BackBattleField, -1)) {
							return 0;
						}//有重复角色在出战席
						std::unique_ptr<BasicObject> tmp = std::move(Zone(OriginPlaceType::FrontBattleField)[i]);
						Zone(OriginPlaceType::FrontBattleField)[i] = std::move(holding);
						Zone(holdoriginplace)[OriginIndex] = std::move(tmp);
						return 1;//成功交换
					}
				}
			}
			for (int i = 0;i < ObjectPlace(OriginPlaceType::BackBattleField).size();i++) {//后台检测
				
				if (PlaceInside(x, y, ObjectPlace(OriginPlaceType::BackBattleField)[i])) {
					if (Zone(OriginPlaceType::BackBattleField)[i] == nullptr) {//放置的逻辑
						if (CountResouceNum(OriginPlaceType::FrontBattleField)
							+ CountResouceNum(OriginPlaceType::BackBattleField) + 1 > CharacterOnBattleFieldLimit) {
							return 0;
						}//超员了
						if (CheckIfBattleFieldDulplicate(OriginPlaceType::FrontBattleField, -1) ||
							CheckIfBattleFieldDulplicate(OriginPlaceType::BackBattleField, i)) {
							return 0;
						}//有重复角色在出战席
						Zone(OriginPlaceType::BackBattleField)[i] = std::move(holding);
						return 1;//成功放置
					}
					else {//交换的逻辑
						if (CheckIfBattleFieldDulplicate(OriginPlaceType::FrontBattleField, -1) ||
							CheckIfBattleFieldDulplicate(OriginPlaceType::BackBattleField, i)) {
							return 0;
						}//有重复角色在出战席
						std::unique_ptr<BasicObject> tmp = std::move(Zone(OriginPlaceType::BackBattleField)[i]);
						Zone(OriginPlaceType::BackBattleField)[i] = std::move(holding);
						Zone(holdoriginplace)[OriginIndex] = std::move(tmp);
						return 1;//成功交换
					}
				}
			}

			//这个其实应该角色/箱子/聘书之类直接fallthrough过来，但目前还没做角色以外的class
			for (int i = 0;i < ObjectPlace(OriginPlaceType::Bench).size();i++) {//备战席检测
				if (PlaceInside(x, y, ObjectPlace(OriginPlaceType::Bench)[i])) {
					if (Zone(OriginPlaceType::Bench)[i] == nullptr) {//避免自己->自己
						if (i + 1 > CharacterOnBenchLimit) {//可能有限制备战席格子的情况
							return 0;
						}
						Zone(OriginPlaceType::Bench)[i] = std::move(holding);//备战席不检查角色重复，因为备战席可能放箱子等非角色
						return 1;
					}
					else {//交换的逻辑：出战席-备战席交换（一定是角色）/备战席-备战席交换（可能不是角色）
						if (holdoriginplace == OriginPlaceType::FrontBattleField || holdoriginplace == OriginPlaceType::BackBattleField) {
							if (holding->Type() != ResourceType::Character) {//非角色不能换
								return 0;
							}
							if (Zone(OriginPlaceType::Bench)[i]->Type() != ResourceType::Character) {
								return 0;
							}//这保证了cast一定安全
							auto* h = static_cast<CharacterObject*>(Zone(OriginPlaceType::Bench)[i].get());
							if (CheckIfBattleFieldDulplicate(OriginPlaceType::FrontBattleField, holdoriginplace == OriginPlaceType::FrontBattleField?static_cast<int>(OriginIndex):-1,h->name) ||
								CheckIfBattleFieldDulplicate(OriginPlaceType::BackBattleField, holdoriginplace == OriginPlaceType::BackBattleField ? static_cast<int>(OriginIndex) : -1,h->name)) {
								return 0;
							}//有重复角色在出战席
							std::unique_ptr<BasicObject> tmp = std::move(Zone(OriginPlaceType::Bench)[i]);
							Zone(OriginPlaceType::Bench)[i] = std::move(holding);
							Zone(holdoriginplace)[OriginIndex] = std::move(tmp);
							return 1;
						}
						else {//备战席-备战席
							std::unique_ptr<BasicObject> tmp = std::move(Zone(OriginPlaceType::Bench)[i]);
							Zone(OriginPlaceType::Bench)[i] = std::move(holding);
							Zone(holdoriginplace)[OriginIndex] = std::move(tmp);
							return 1;//成功交换
						}
					}
				}
			}
			break;
		case ResourceType::Equipment:
			break;
		}
		return 0;
	}
	void Logic::PutBack() {
		if (!holding) {
			return;
		}
		Zone(holdoriginplace)[OriginIndex] = std::move(holding);
	}
	void Logic::LongPress() {

	}
	void Logic::Click() {
		//UI按钮检测
		Log("Click"+std::to_string(MotionX)+":"+std::to_string(MotionY), __func__, Logcode::OK);
		auto uiinside = [&](const std::string& s) {
			return PlaceInside(PressStartX, PressStartY, buttonplace[s]);
			};
		
		if (Rmode == RealTimeMode::Shop) {
			if (uiinside("Shop_CloseShopButton")) {
				Rmode = RealTimeMode::Normal;
				return;
			}
			else if (uiinside("Shop_DcardButton")) {
				Log("Dcard", __func__);
				shop.Dcard(MoneyNum);
				return;
			}
			auto& plc = ObjectPlace(OriginPlaceType::ShopCharacter);
			for (int i = 0;i < plc.size();i++) {
				if (PlaceInside(PressStartX, PressStartY, plc[i])) {
					Log("Buy", __func__);
					shop.Buy(MoneyNum, i, Zone(OriginPlaceType::Bench), CharacterOnBenchLimit);
					StarUp();
					return;
				}
			}
			Rmode = RealTimeMode::Normal;
			
		}
		else if (Rmode==RealTimeMode::DetailInfo) {

		}
		else {
			Rmode = RealTimeMode::Normal;
		}
		if (uiinside("Button_Exit")) {
			exit(0);
		}
		if (uiinside("ContinueWin")) {

		}
		if (uiinside("BuyExp")) {
			Log("BuyExp", __func__);
			shop.BuyExp(MoneyNum);
			if (shop.LevelUp()) {
				++CharacterOnBattleFieldLimit;
				Log("LevelUp", __func__);
			}
		}
		if (uiinside("OpenShop")) {
			Rmode = RealTimeMode::Shop;
		}
		if (uiinside("StartFight")) {//退出战斗的逻辑，现在该写入数据交换体了
			DataHandOff();
			AtWaitingForFight = false;
		}
		
	}
	void Logic::DataHandOff() {
		using StageData::g_WaitingForFight_to_Fight;
		using StageData::g_WaitingForFight_publicData;
		g_WaitingForFight_to_Fight.BackCharacter = &Zone(OriginPlaceType::BackBattleField);
		g_WaitingForFight_to_Fight.FrontCharacter = &Zone(OriginPlaceType::FrontBattleField);
		g_WaitingForFight_to_Fight.Enemys = &Enemys;
		g_WaitingForFight_publicData.dimension_index = &dimension_index;
		g_WaitingForFight_publicData.stage_index = &stage_index;
		g_WaitingForFight_to_Fight.difficulty = Difficulty;
	}
	bool Logic::StarUp() {
		// cost=1,2 最高 4 星；cost>=3 最高 3 星
		auto MaxStar = [](int cost) -> int {
			return cost <= 2 ? 4 : 3;
			};

		auto AsCharacter = [](const std::unique_ptr<BasicObject>& obj) -> CharacterObject* {
			if (!obj || obj->Type() != ResourceType::Character) {
				return nullptr;
			}
			return static_cast<CharacterObject*>(obj.get());
			};

		auto CanStarUp = [&](const CharacterObject* c) -> bool {
			if (!c) return false;
			return c->game_attribute.stars < MaxStar(c->game_attribute.cost);
			};

		auto TryOnce = [&]() -> bool {
			// 1. 优先考虑出战席：前台、后台
			const OriginPlaceType battleFields[] = {
				OriginPlaceType::FrontBattleField,
				OriginPlaceType::BackBattleField
			};

			for (OriginPlaceType place : battleFields) {
				auto& zone = Zone(place);
				for (size_t i = 0; i < zone.size(); ++i) {
					CharacterObject* keeper = AsCharacter(zone[i]);
					if (!CanStarUp(keeper)) continue;

					const std::string& name = keeper->name;
					const int star = keeper->game_attribute.stars;

					// 从备战席找两个同 name 同 star 的角色丢弃
					std::vector<size_t> discard;
					auto& bench = Zone(OriginPlaceType::Bench);
					for (size_t j = 0; j < bench.size() && discard.size() < 2; ++j) {
						CharacterObject* c = AsCharacter(bench[j]);
						if (c && c->name == name && c->game_attribute.stars == star) {
							discard.push_back(j);
						}
					}

					if (discard.size() >= 2) {
						keeper->game_attribute.stars += 1;
						bench[discard[0]].reset();
						bench[discard[1]].reset();
						return true;
					}
				}
			}

			// 2. 三个都在备战席：保留最左边的角色
			auto& bench = Zone(OriginPlaceType::Bench);
			for (size_t i = 0; i < bench.size(); ++i) {
				CharacterObject* keeper = AsCharacter(bench[i]);
				if (!CanStarUp(keeper)) continue;

				const std::string& name = keeper->name;
				const int star = keeper->game_attribute.stars;

				std::vector<size_t> same;
				for (size_t j = 0; j < bench.size(); ++j) {
					CharacterObject* c = AsCharacter(bench[j]);
					if (c && c->name == name && c->game_attribute.stars == star) {
						same.push_back(j);
					}
				}

				// 保留最左边：当前 i 必须是同组中索引最小的
				if (same.size() >= 3 && same.front() == i) {
					keeper->game_attribute.stars += 1;
					bench[same[1]].reset();
					bench[same[2]].reset();
					return true;
				}
			}

			return false;
			};

		bool changed = false;
		while (TryOnce()) {
			changed = true;
		}
		return changed;
	}
	void Logic::Init_Enemys() {
		Enemys.clear();

		// Boss 必须在 Init 之前已经加载好
		if (Boss.empty()) {
			return;
		}

		// 随机选择一个 Boss
		const EnemyObject& boss = Boss[GetRandomNum(0, static_cast<int>(Boss.size()) - 1)];

		std::vector<std::string> elitePool;
		std::vector<std::string> normalPool;

		// 根据 Boss 的 mobs 分类
		for (const auto& mobName : boss.mobs) {
			auto it = Object::Enemy_DefaultSetting.find(mobName);
			if (it == Object::Enemy_DefaultSetting.end()) {
				continue;
			}

			const std::string& type = it->second.EnemyType;
			if (type == "elite") {
				elitePool.push_back(mobName);
			}
			else if (type == "normal") {
				normalPool.push_back(mobName);
			}
		}

		// 数据不足时直接返回，避免生成错误
		if (elitePool.empty() || normalPool.empty()) {
			return;
		}

		// 随机 1~2 个精英，剩下 8~9 个普通，总数恒为 10
		int eliteCount = GetRandomNum(1, 2);
		int normalCount = 10 - eliteCount;

		// 加入精英怪
		for (int i = 0; i < eliteCount; ++i) {
			int idx = GetRandomNum(0, static_cast<int>(elitePool.size()) - 1);
			Enemys.push_back(Object::Enemy_DefaultSetting.at(elitePool[idx]));
		}

		// 加入普通怪
		for (int i = 0; i < normalCount; ++i) {
			int idx = GetRandomNum(0, static_cast<int>(normalPool.size()) - 1);
			Enemys.push_back(Object::Enemy_DefaultSetting.at(normalPool[idx]));
		}
	}
	void Logic::Init_Boss() {

	}
	void Logic::Init() {
		int m = 20;
		State.shop.Dcard(m, 1);
		State.AtWaitingForFight = true;
		// 先检查维度是否越界
		if (dimension_index >= stage.size()) {
			// 游戏结束，这里暂时用 exit(0)，以后可以换成 GameEnd()
			exit(0);
		}

		// 再检查当前维度内的阶段索引是否越界
		if (stage_index >= stage[dimension_index].size()) {
			// 尝试进入下一个维度
			if (dimension_index + 1 < stage.size()) {
				stage_index = 0;
				++dimension_index;
				// 此时 dimension_index 一定小于 stage.size()，无需再检查
			}
			else {
				// 没有下一个维度了，游戏结束
				exit(0);
			}
		}
		enum StageType {
			Bonus,
			Normal,
			Supply,
			Strategy,
			Encounter,
			Boss
		};
		static std::unordered_map<std::string, StageType> typereflect = {
			{"Bonus",Bonus},
			{"Normal",Normal},
			{"Supply",Supply},
			{"Strategy",Strategy},
			{"Encounter",Encounter},
			{"Boss",Boss}
		};
		switch (typereflect[stage[dimension_index][stage_index]]) {
		case Bonus:
			Init_Enemys();
			break;
		case Normal:
			Init_Enemys();
			break;
		case Supply:
			Init_Enemys();
			break;
		case Strategy:
			Init_Enemys();
			break;
		case Encounter:
			Init_Enemys();
			break;
		case Boss:
			Init_Boss();
			break;
		default:
			break;
		}
	}
}