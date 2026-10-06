#include"Fight_Logic.h"
#include"StageData.h"
#include"SDL_Functions.h"
#include"Logger.h"
#include"json.hpp"
#include<algorithm>
#include<fstream>
#include<sstream>
using Logger::Log;
using json = nlohmann::json;
namespace Fight {
	Fight_Logic State;
	void Fight_Logic::BuildActionList() {
		ActionList.clear();
		for (auto& var : actors) {
			for (auto& act : var) {
				if (!act || act->id() == -1 || act->Speed == 0) {
					continue;
				}
				act->ActionValue = ActionValueInaRound / act->Speed;
				ActionList.push_back({ act->id(),act->ActionValue });
			}

		}
		sort(ActionList.begin(), ActionList.end());
	}
	void Fight_Logic::AddtoActionList(ActorList act) {

	}
	void Fight_Logic::AddtoExtraAction(ExtraActorEntry act) {
		if (act.id == -1)return;
		auto it = ExtraAction.begin();
		while (it != ExtraAction.end()
			&& static_cast<int>(it->Priority) <= static_cast<int>(act.Priority)) {
			++it;
		}
		ExtraAction.insert(it, act);
	}
	void Fight_Logic::RemoveFromActionList_id(int id) {
		if (id == -1) {
			return;
		}
		for (auto it = ActionList.begin();it != ActionList.end();) {
			if (id == it->id) {
				it = ActionList.erase(it);
			}
			else {
				++it;
			}
		}
	}
	void Fight_Logic::RemoveFromActionList_index(size_t index) {
		ActionList.erase(ActionList.begin() + index);
	}
	void Fight_Logic::RemoveFromExtraList_id(int id) {
		if (id == -1)return;
		for (auto it = ExtraAction.begin();it != ExtraAction.end();) {
			if (id == it->id) {
				it = ExtraAction.erase(it);
			}
			else {
				++it;
			}
		}
	}
	void Fight_Logic::RemoveFromExtraList_index(size_t index) {
		ExtraAction.erase(ExtraAction.begin() + index);
	}
	void Fight_Logic::LoadinJson(const std::filesystem::path& gp) {
		std::ifstream configin(gp / "config" / "Fight_Logic.json");//从配置读入
		if (!configin.is_open()) {
			Log("Failed to open Fight_Logic.json", __FUNCSIG__, Logcode::OPENFAILED);
			return;
		}
		json js;
		try {
			configin >> js;
		}
		catch (const std::exception& e) {
			Log("Failed to parse Fight_Logic.json", __FUNCSIG__, Logcode::PARSEERROR);
		}
		auto StateTypeFromString = [](const std::string& str) {
			if (str == "Idle")return StateType::Idle;
			else if (str == "EnemyStart")return StateType::EnemyStart;
			else if (str == "EnemyPreparing")return StateType::EnemyPreparing;
			else if (str == "EnemyExecuting")return StateType::EnemyExecuting;
			else if (str == "PlayerStart")return StateType::PlayerStart;
			
			else if (str == "PlayerExecuting")return StateType::PlayerExecuting;
			else return StateType::Idle;
			};
		if (js.contains("Animation_LogicDelay") && js["Animation_LogicDelay"].is_object()) {
			for (auto& [key, value] : js["Animation_LogicDelay"].items()) {
				if (value.is_number_unsigned()) {
					Animation_LogicDelay[StateTypeFromString(key)] = value.get<Uint64>();
				}
			}
		}
		if (js.contains("SkillPoint") && js["SkillPoint"].is_number_integer()) {
			SkillPoint = js["SkillPoint"].get<int>();
		}
		if (js.contains("SkillPointMax") && js["SkillPointMax"].is_number_integer()) {
			SkillPointMax = js["SkillPointMax"].get<int>();
		}

		//从配置文件中读取UI位置
		std::ifstream uiConfigIn(gp / "data" / "Fight_Texture.json");
		json uiJs;
		if (!uiConfigIn.is_open()) {
			Log("Failed to open Fight_Texture.json", __FUNCSIG__, Logcode::OPENFAILED);
			return;
		}
		ui_place.clear();
		auto ParseFRect = [](const std::string& str) {
			SDL_FRect rect{ 0,0,0,0 };
			std::istringstream iss(str);
			iss >> rect.x >> rect.y >> rect.w >> rect.h;
			return rect;
			};
		try {
			uiConfigIn >> uiJs;
		}
		catch (const std::exception& e) {
			Log("Failed to parse Fight_Texture.json", __FUNCSIG__, Logcode::PARSEERROR);
		}
		if (uiJs.contains("ui") && uiJs["ui"].is_array()) {
			for (auto& item : uiJs["ui"]) {
				if (!item.contains("type"))continue;
				std::string type = item["type"].get<std::string>();
				if (!item.contains(type))continue;
				for (const auto& var : item.at(type)) {
					ui_place[type].push_back(ParseFRect(var.value("place", "0 0 0 0")));
				}
			}
		}
	}
	void Fight_Logic::Log_Init() {
		for (auto& vec : actors) {
			for (auto& var : vec) {
				Log(std::to_string(var->id()) 
					+":" 
					+ var->Name()+" "
					+std::to_string(var->Speed)
					, __func__);
				auto* p = dynamic_cast<Character_Actor*>(var.get());
				if (p)
					Log(std::to_string(p->config.basic_attribute.Level), __func__);
			}
		}
	}
	void Fight_Logic::Init() {
		Timer = 0;
		AtFight = FightGlobalState::AtFight;
		GlobalIdIndex = 0;
		using StageData::g_WaitingForFight_to_Fight;
		for (auto& var : actors) {
			var.clear();//把上一次战斗所有内容都清除
		}
		
		for (auto& var : *(g_WaitingForFight_to_Fight.FrontCharacter)) {
			if (auto* charObjPtr = dynamic_cast<CharacterObject*>(var.get())) {
				auto tmp = std::make_unique<Character_Actor>();
				tmp->NewActor(*charObjPtr); // 传入解引用后的对象
				Actor(PlaceType::FrontCharacter).push_back(std::move(tmp));
			}
		}
		for (auto& var : *(g_WaitingForFight_to_Fight.BackCharacter)) {
			if (auto* charObjPtr = dynamic_cast<CharacterObject*>(var.get())) {
				auto tmp = std::make_unique<Character_Actor>();
				tmp->NewActor(*charObjPtr); // 传入解引用后的对象
				Actor(PlaceType::BackCharacter).push_back(std::move(tmp));
			}
		}
		for (auto& var : *(g_WaitingForFight_to_Fight.Enemys)) {
			auto tmp = std::make_unique<Enemy_Actor>();
			tmp->NewActor(var.name,g_WaitingForFight_to_Fight.difficulty);
			Actor(PlaceType::Enemy).push_back(std::move(tmp));
		}
		//接下来给所有Actor附上唯一编号
		for (auto& vec : actors) {
			for (auto& var : vec) {
				var->SetId(GlobalIdIndex++);
			}
		}
		//Log_Init();
	}
	void Fight_Logic::DispatchEvent() {
		auto HandleGlobalEvent = [&](const SDL_Event& event)->bool {
			switch (event.type) {
			case SDL_EVENT_KEY_DOWN:
				if (event.key.scancode == SDL_SCANCODE_ESCAPE) {
					AtFight = FightGlobalState::Pause;
					return 1;
				}
				
				break;
			case SDL_EVENT_QUIT:
				AtFight = FightGlobalState::Pause;
				return 1;
				break;
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
				if (event.button.button == SDL_BUTTON_LEFT) {
					if (PlaceInside(event.button.x, event.button.y, ui_place["Quit"][0])) {
						AtFight = FightGlobalState::Pause;
						return 1;
					}
				}
				break;
			}
			return 0;
			};
		auto CheckIfUlt = [&](float x, float y)->bool {//坐标版，是否点击了大招并处理
			for (size_t i = 0;i < ui_place["Ult"].size();i++) {
				if (PlaceInside(x, y, ui_place["Ult"][i])) {
					auto* p = dynamic_cast<Character_Actor*>(Actor(PlaceType::FrontCharacter)[i].get());
					if (p->Energy >= p->config.basic_attribute.EnergyLimit) {
						AddtoExtraAction({ p->id(),ExtraPriority::CharacterUlt });
						p->Energy = 0;
						return 1;
					}
				}
			}
			return 0;
			};
		auto HandleMouseClickEvent = [&](float x, float y) {
			switch (CurrentState) {
			case StateType::Idle:
				CheckIfUlt(x, y);
				break;
			case StateType::PlayerStart:
				if (CheckIfUlt(x, y)) {
					AddtoExtraAction({ NowActing->id(),ExtraPriority::CharacterEnforced });
					CurrentState = StateType::PlayerUlt;
					NowActing = FindActorFromExtraAction(0);
				}
			}
			};
		for (auto& event : EventQueue) {
			if (HandleGlobalEvent(event)) {
				continue;
			}
			switch (event.type) {
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
				if (event.button.button == SDL_BUTTON_LEFT) {
					//处理点击事件
					HandleMouseClickEvent(event.button.x,event.button.y);
				}
				break;
			}
		}
		
	}
	void Fight_Logic::PollEvents() {
		EventQueue.clear();
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			EventQueue.push_back(event);
			

		}
	}
	BasicActor* Fight_Logic::FindActorFromActionList(size_t index) {
		auto& wact = ActionList[index];
		for (auto& var : actors) {
			for (auto& act : var) {
				if (act->id() == wact.id) {
					return act.get();
				}
			}
		}
		return nullptr;
	}
	BasicActor* Fight_Logic::FindActorFromExtraAction(size_t index) {
		auto& wact = ExtraAction[index];
		for (auto& var : actors) {
			for (auto& act : var) {
				if (act->id() == wact.id) {
					return act.get();
				}
			}
		}
		return nullptr;
	}
	void Fight_Logic::StartActor() {
		auto StateShifter = [&](BasicActor* act) {
			switch (act->Type()) {
			case ActorType::Character:
				CurrentState = StateType::PlayerStart;
				break;
			case ActorType::Enemy:
				CurrentState = StateType::EnemyStart;
				break;
			}
			
			};
		if (!ExtraAction.empty()) {
			NowActing = FindActorFromExtraAction(0);
			RemoveFromExtraList_index(0);
			StateShifter(NowActing);
		}
		else if (!ActionList.empty()) {
			NowActing = FindActorFromActionList(0);
			RemoveFromActionList_index(0);
			StateShifter(NowActing);
		}
		else {
			BuildActionList();
		}
	}
	void Fight_Logic::Tick(Uint64 dt) {
		switch (CurrentState) {
		case StateType::Idle:
			StartActor();
			Timer = 0;
			break;
		case StateType::PlayerStart:
			Timer += dt;
			if (Timer >= Animation_LogicDelay[StateType::PlayerStart]) {
				CurrentState = StateType::PlayerSelecting;
				Timer = 0;
			}
				
			break;
		case StateType::PlayerSelecting:
			Timer += dt;
			break;
		case StateType::EnemyStart:
			Timer += dt;
			if (Timer >= Animation_LogicDelay[StateType::EnemyStart]) {
				EnemyStart_Resolution();
				CurrentState = StateType::EnemyPreparing;
				Timer = 0;
			}
			break;
		case StateType::EnemyPreparing:
			Timer += dt;
			if (Timer >= Animation_LogicDelay[StateType::EnemyPreparing]) {
				EnemyPreparing();
				CurrentState = StateType::EnemyExecuting;
				Timer = 0;
			}
			break;
		case StateType::EnemyExecuting:
			Timer += dt;
			if (Timer >= Animation_LogicDelay[StateType::EnemyExecuting]) {
				CurrentState = StateType::Idle;
				Timer = 0;
			}
			break;
		}
	}
	void Fight_Logic::EnemyStart_Resolution() {
		
	}
	void Fight_Logic::EnemyPreparing() {

	}
	void Fight_Logic::EnemyExecuting() {

	}
}