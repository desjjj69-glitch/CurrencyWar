#include"Fight_Logic.h"
#include"StageData.h"
#include"SDL_Functions.h"
#include"Logger.h"
#include"json.hpp"
#include<algorithm>
#include<fstream>
#include<sstream>
#include"BasicGameResource.h"
using Logger::Log;
using json = nlohmann::json;
namespace Fight {
	Fight_Logic State;
	void Fight_Logic::Pause() {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_EVENT_QUIT:
				exit(0);
				break;
			case SDL_EVENT_KEY_DOWN:
				Log("Key down scancode: " + std::to_string(event.key.scancode), __func__);
				if (event.key.scancode == SDL_SCANCODE_ESCAPE)
					AtFight = FightGlobalState::EndFight;
				else if (event.key.scancode == SDL_SCANCODE_SPACE) {
					AtFight = FightGlobalState::AtFight;
				}
				break;
			}
		}
	}
	void Fight_Logic::BuildActionList() {
		ActionList.clear();
		ExtraAction.clear();
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
		if (act.id == -1)
			return;
		auto it = ActionList.begin();
		for (auto& var : ActionList) {
			if (var.id == act.id)
				return;
		}
		while (it != ActionList.end() && it->ActionValue <= act.ActionValue) {
			it++;
		}
		ActionList.insert(it, act);
	}
	void Fight_Logic::AddtoActionList(BasicActor* act) {
		if (!act || act->id() == -1 || act->Speed == 0) {
			return;
		}
		act->ActionValue = ActionValueInaRound / act->Speed;
		AddtoActionList({ act->id(),act->ActionValue });
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
		if (ActionList.size() > index) {
			ActionList.erase(ActionList.begin() + index);
		}
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
		if (ExtraAction.size() > index) {
			ExtraAction.erase(ExtraAction.begin() + index);
		}
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
			std::string err = "Failed to parse Fight_Logic.json";
			err+=e.what();
			Log(err, __FUNCSIG__, Logcode::PARSEERROR);
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
			std::string err = "Failed to parse Fight_Texture.json";
			err += e.what();
			Log(err, __FUNCSIG__, Logcode::PARSEERROR);
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
		auto CheckIfUltbyPos = [&](float x, float y)->bool {//坐标版，是否点击了大招并处理
			for (size_t i = 0;i < ui_place["Ult"].size();i++) {
				if (PlaceInside(x, y, ui_place["Ult"][i])) {
					if(i>=Actor(PlaceType::FrontCharacter).size())
						continue;
					auto* p = dynamic_cast<Character_Actor*>(Actor(PlaceType::FrontCharacter)[i].get());
					if (p->Energy >= p->config.basic_attribute.EnergyLimit&&p->heart>0) {
						AddtoExtraAction({ p->id(),ExtraPriority::CharacterUlt });
						p->Energy = 0;
						return 1;
					}
				}
			}
			return 0;
			};
		auto CheckIfUltbyKey = [&](const SDL_KeyboardEvent& key) {//键盘版，是否按下了大招并处理
			for (size_t i = 0;i < ui_place["Ult"].size();i++) {
				if (key.scancode == SDL_SCANCODE_1 + i) {
					if (i >= Actor(PlaceType::FrontCharacter).size())
						continue;
					auto* p = dynamic_cast<Character_Actor*>(Actor(PlaceType::FrontCharacter)[i].get());
					if (p->Energy >= p->config.basic_attribute.EnergyLimit && p->heart > 0) {
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
				CheckIfUltbyPos(x, y);
				break;
			case StateType::PlayerStart:
				if (CheckIfUltbyPos(x, y)) {
					AddtoExtraAction({ NowActing->id(),ExtraPriority::CharacterEnforced });
					CurrentState = StateType::PlayerUlt;
					NowActing = FindActorFromExtraAction(0);
					return;
				}
				break;
			case StateType::PlayerSelecting:
				if (CheckIfUltbyPos(x, y)) {//如果这次点击的是大招按钮，那么就把当前的选择行为强制结束
					return;
				}
				switch (CurrentCharacterState) {
				case CharacterState::Normal:
					if (PlaceInside(x, y, ui_place["Skill"][0])) {
						CurrentCharacterState = CharacterState::SKill;
					}
					break;
				case CharacterState::SKill:
					if (PlaceInside(x, y, ui_place["Normal"][0])) {
						CurrentCharacterState = CharacterState::Normal;
					}
					break;
				}
				break;
			case StateType::PlayerExecuting:
				CheckIfUltbyPos(x, y);
				break;
			case StateType::EnemyStart:
				if (CheckIfUltbyPos(x, y)) {
					AddtoExtraAction({ NowActing->id(),ExtraPriority::EnemyEnforced });
					CurrentState = StateType::PlayerUlt;
					NowActing = FindActorFromExtraAction(0);
					return;
				}
				break;
			case StateType::EnemyPreparing:
				CheckIfUltbyPos(x, y);
				break;
			case StateType::EnemyExecuting:
				CheckIfUltbyPos(x, y);
				break;
			case StateType::PlayerUlt:
				CheckIfUltbyPos(x, y);
				break;
			case StateType::PlayerUltSelecting:
				CheckIfUltbyPos(x, y);
				break;
			case StateType::PlayerUltExecuting:
				CheckIfUltbyPos(x, y);
				break;
			}
			
				
			};
		auto HandleKeyBoardEvent = [&](const SDL_KeyboardEvent &key) {
			switch (CurrentState) {
			case StateType::Idle:
				CheckIfUltbyKey(key);
				break;
			case StateType::PlayerStart:
				if (CheckIfUltbyKey(key)) {
					AddtoExtraAction({ NowActing->id(),ExtraPriority::CharacterEnforced });
					CurrentState = StateType::PlayerUlt;
					NowActing = FindActorFromExtraAction(0);
					return;
				}
				break;
			case StateType::PlayerSelecting:
				if (CheckIfUltbyKey(key)) {//如果这次点击的是大招按钮，那么就把当前的选择行为强制结束
					return;
				}
				switch (CurrentCharacterState) {
				case CharacterState::Normal:
					if (key.scancode == SDL_SCANCODE_E) {
						CurrentCharacterState = CharacterState::SKill;
					}
					break;
				case CharacterState::SKill:
					if (key.scancode == SDL_SCANCODE_Q) {
						CurrentCharacterState = CharacterState::Normal;
					}
					break;
				}
				break;
			case StateType::PlayerExecuting:
				CheckIfUltbyKey(key);
				break;
			case StateType::EnemyStart:
				if (CheckIfUltbyKey(key)) {
					AddtoExtraAction({ NowActing->id(),ExtraPriority::EnemyEnforced });
					CurrentState = StateType::PlayerUlt;
					NowActing = FindActorFromExtraAction(0);
					return;
				}
				break;
			case StateType::EnemyPreparing:
				CheckIfUltbyKey(key);
				break;
			case StateType::EnemyExecuting:
				CheckIfUltbyKey(key);
				break;
			case StateType::PlayerUlt:
				CheckIfUltbyKey(key);
				break;
			case StateType::PlayerUltSelecting:
				CheckIfUltbyKey(key);
				break;
			case StateType::PlayerUltExecuting:
				CheckIfUltbyKey(key);
				break;
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
			case SDL_EVENT_KEY_DOWN:
				HandleKeyBoardEvent(event.key);
			}
		}
		
	}
	void Fight_Logic::PollEvents() {
		EventQueue.clear();
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			SDL_ConvertEventToRenderCoordinates(BasicGameRes.GameRenderer, &event);
			EventQueue.push_back(event);
		}
	}
	BasicActor* Fight_Logic::FindActorFromActionList(size_t index) {
		if (index >= ActionList.size())
			return nullptr;
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
		if (index >= ExtraAction.size())
			return nullptr;
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
		case StateType::PlayerUlt:
			Timer += dt;
			CurrentCharacterState = CharacterState::Ultimate;
			if (Timer >= Animation_LogicDelay[StateType::PlayerUlt]) {
				CurrentState = StateType::PlayerUltSelecting;
				Timer = 0;
			}
			break;
		case StateType::PlayerUltSelecting:
			Timer += dt;
			break;
		case StateType::PlayerUltExecuting:
			Timer += dt;
			if (Timer >= Animation_LogicDelay[StateType::PlayerUltExecuting]) {
				CurrentState = StateType::Idle;
				Timer = 0;
			}
			break;
			
		case StateType::PlayerSelecting:
			Timer += dt;
			break;
		case StateType::PlayerExecuting:
			Timer += dt;
			if (Timer >= Animation_LogicDelay[StateType::PlayerExecuting]) {
				AddtoActionList(NowActing);
				CurrentState = StateType::Idle;
				Timer = 0;
			}
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
				EnemyExecuting();
				AddtoActionList(NowActing);
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
		if (!NowActing)
			return;
		Log(NowActing->Name(), __func__);
		auto* skill = NowActing->GetSkillByIndex(0);//技能暂时写死
		if (!skill)
			return;
		SkillTargets targets;//技能目标暂时写死
		auto& front = Actor(PlaceType::FrontCharacter);
		if (!front.empty()) {
			targets.list.push_back({ front[0].get(), TargetRole::Main, 1.0 });
		}

		skill->Execute(NowActing, targets, *this);
	}
	void Fight_Logic::CharacterStart_Resolution(Character_Actor* act) {
		
	}
	void Fight_Logic::DealDamage(BasicActor* caster, BasicActor* target,
		const Attribute::AttackForm& form,
		double multiplier) {
		if (!caster || !target)
			return;
		Log("DealDamage: " + caster->Name()
			+ " -> " + target->Name()
			+ " skill=" + form.id
			+ " mult=" + std::to_string(multiplier),
			__func__);//测试用
	}
	void Fight_Logic::GainEnergy(BasicActor* who, double amount) {

	}
	void Fight_Logic::ApplyBuff(BasicActor* target, const std::string& buffId) {

	}
}