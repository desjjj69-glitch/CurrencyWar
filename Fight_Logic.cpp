#include"Fight_Logic.h"
#include"StageData.h"
#include"SDL_Functions.h"
#include"Logger.h"
#include"json.hpp"
#include"GetRandomNum.h"
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
		act->ActionValue += ActionValueInaRound / act->Speed;
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
		if (js.contains("DefaultSkillPoint") && js["DefaultSkillPoint"].is_number_integer()) {
			DefaultSkillPoint = js["DefaultSkillPoint"].get<int>();
		}
		if (js.contains("DefaultSkillPointMax") && js["DefaultSkillPointMax"].is_number_integer()) {
			DefaultSkillPointMax = js["DefaultSkillPointMax"].get<int>();
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
		SkillPoint = DefaultSkillPoint;
		SkillPointMax = DefaultSkillPointMax;
		using StageData::g_WaitingForFight_to_Fight;
		for (auto& var : actors) {
			var.clear();//把上一次战斗所有内容都清除
		}
		ActionList.clear();
		ExtraAction.clear();
		EnemyActorPool.clear();
		CurrentState = StateType::Idle;
		CurrentCharacterState = CharacterState::Normal;
		NowActing = nullptr;
		Now_skill = nullptr;
		skill_targets.list.clear();


		//接下来才是新写入
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
			EnemyActorPool.push_back(std::move(tmp));//先把敌人写进池子
		}
		for (auto& var : EnemyActorPool) {
			if (!var)continue;
			auto& enemyact = Actor(PlaceType::Enemy);
			if (enemyact.size() < EnemyOnStageNumLimit) {
				enemyact.push_back(std::move(var));
			}
		}
		//接下来给所有Actor附上唯一编号
		for (auto& vec : actors) {
			for (auto& var : vec) {
				var->SetId(GlobalIdIndex++);
			}
		}
		//Log_Init();
	}
	void Fight_Logic::ShiftCharacterMainTarget(int delta) {
		if (!Now_skill) return;

		auto& enemies = Actor(PlaceType::Enemy);

		// 收集非空位置（这就自然跳过了 nullptr）
		std::vector<size_t> valid;
		valid.reserve(enemies.size());
		for (size_t i = 0; i < enemies.size(); ++i)
			if (enemies[i]) valid.push_back(i);
		if (valid.empty()) return;

		// 找到当前主目标在 valid 中的下标
		int cur = -1;
		if (auto* mainActor = skill_targets.Main()) {
			for (int i = 0; i < static_cast<int>(valid.size()); ++i) {
				if (enemies[valid[i]].get() == mainActor) {
					cur = i;
					break;
				}
			}
		}
		// 没有主目标（比如之前没设置过），退回中间
		if (cur < 0) cur = static_cast<int>(valid.size() / 2);

		int next = cur + delta;
		if (next < 0) next = 0;
		if (next >= static_cast<int>(valid.size()))
			next = static_cast<int>(valid.size()) - 1;

		// 用新的 mainIndex 重建 targets（AoE 情况下其实主目标意义不大，但重建无害）
		BuildTargetsByMainIndex(enemies, valid[next], Now_skill->form.target, skill_targets);
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
						SetCharacterSkillBySlot(Attribute::SkillSlot::Skill);
						CurrentCharacterState = CharacterState::SKill;
					}
					else if (PlaceInside(x, y, ui_place["Normal"][0])) {
						CurrentState = StateType::PlayerExecuting;
						SkillPoint = std::max(++SkillPoint, SkillPointMax);
					}
					break;
				case CharacterState::SKill:
					if (PlaceInside(x, y, ui_place["Normal"][0])) {
						SetCharacterSkillBySlot(Attribute::SkillSlot::Normal);
						CurrentCharacterState = CharacterState::Normal;
					}
					if (PlaceInside(x, y, ui_place["Skill"][0])) {
						if (SkillPoint <= 0) {
							break;
						}
						CurrentState = StateType::PlayerExecuting;
						SkillPoint--;
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
				if (key.scancode == SDL_SCANCODE_LEFT) {
					ShiftCharacterMainTarget(-1);
					return;
				}
				if (key.scancode == SDL_SCANCODE_RIGHT) {
					ShiftCharacterMainTarget(+1);
					return;
				}
				switch (CurrentCharacterState) {
				case CharacterState::Normal:
					if (key.scancode == SDL_SCANCODE_E) {
						SetCharacterSkillBySlot(Attribute::SkillSlot::Skill);
						CurrentCharacterState = CharacterState::SKill;
					}
					else if (key.scancode == SDL_SCANCODE_Q || key.scancode == SDL_SCANCODE_SPACE) {
						CurrentState = StateType::PlayerExecuting;
						
						SkillPoint = std::max(++SkillPoint, SkillPointMax);
					}
					break;
				case CharacterState::SKill:
					if (key.scancode == SDL_SCANCODE_Q) {
						SetCharacterSkillBySlot(Attribute::SkillSlot::Normal);
						CurrentCharacterState = CharacterState::Normal;

					}
					if (key.scancode == SDL_SCANCODE_E || key.scancode == SDL_SCANCODE_SPACE) {
						if (SkillPoint <= 0) {
							break;
						}
						CurrentState = StateType::PlayerExecuting;
						SkillPoint--;
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
				CharacterStart_Resolution(dynamic_cast<Character_Actor*>(NowActing));
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
				CharacterExecuting();
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
		//在一次行动之后，要把已经死亡的角色/敌人移除行动条，并可能加入新的敌人
		for (auto& var : Actor(PlaceType::FrontCharacter)) {
			auto* p = dynamic_cast<Character_Actor*>(var.get());
			if (!p)continue;
			if (p->heart <= 0) {
				RemoveFromExtraList_id(p->id());
				RemoveFromActionList_id(p->id());
				var.reset();
			}
		}
		auto GetEnemyFromPool = [&]() ->std::unique_ptr<BasicActor>{
			for (auto& var : EnemyActorPool) {
				if (var) {
					return std::move(var);
				}
			}
			return nullptr;
			};
		for (auto& var : Actor(PlaceType::Enemy)) {
			auto* p = dynamic_cast<Enemy_Actor*>(var.get());
			if (!p)continue;
			if (p->CurrentHeart <= 0) {
				RemoveFromActionList_id(p->id());
				RemoveFromExtraList_id(p->id());
				var.reset();
				var = GetEnemyFromPool();
			}
		}
		//先检测敌人是不是打完，如果打完，哪怕角色都没了也能赢
		int flag = -1;
		for (auto& var : Actor(PlaceType::Enemy)) {
			if (var) {
				flag = 0;
				break;
			}
		}
		if (flag == -1) {
			DataHandOff(1);
			AtFight = FightGlobalState::EndFight;
		}
		for (auto& var : Actor(PlaceType::FrontCharacter)) {
			if (var) {
				flag = 1;
			}
		}
		if (flag == 0) {
			DataHandOff(0);
			AtFight = FightGlobalState::EndFight;
		}
	}
	void Fight_Logic::EnemyStart_Resolution() {
		
	}
	void Fight_Logic::BuildTargetsByMainIndex(
		const std::vector<std::unique_ptr<BasicActor>>& enemies,
		size_t mainIndex,
		Attribute::TargetAttackType type,
		SkillTargets& out)
	{
		out.list.clear();
		if (enemies.empty()) return;

		auto pushTarget = [&](size_t index, TargetRole role, double multiplier = 1.0) {
			if (index < enemies.size() && enemies[index]) {
				out.list.push_back({ enemies[index].get(), role, multiplier });
			}
			};

		switch (type) {
		case Attribute::TargetAttackType::Single:
			pushTarget(mainIndex, TargetRole::Main);
			break;

		case Attribute::TargetAttackType::AoE:
			for (size_t i = 0; i < enemies.size(); ++i)
				pushTarget(i, TargetRole::Main);
			break;

		case Attribute::TargetAttackType::Bounce:
			pushTarget(mainIndex, TargetRole::Main);
			for (size_t i = 0; i < enemies.size(); ++i) {
				if (i == mainIndex) continue;
				pushTarget(i, TargetRole::Sub);
			}
			break;

		case Attribute::TargetAttackType::Blast:
			pushTarget(mainIndex, TargetRole::Main);
			if (mainIndex > 0)                  pushTarget(mainIndex - 1, TargetRole::Adjacent);
			if (mainIndex + 1 < enemies.size()) pushTarget(mainIndex + 1, TargetRole::Adjacent);
			break;

		default:
			break;
		}
	}
	void Fight_Logic::EnemyPreparing() {//选择技能和目标
		auto& front = Actor(PlaceType::FrontCharacter);
		if (!NowActing)return;
		if (NowActing->Type() != ActorType::Enemy)return;
		auto* p = static_cast<Enemy_Actor*>(NowActing);
		auto& skill_list = p->Skills();
		Now_skill = nullptr;
		skill_targets.list.clear();//清理掉之前的残留
		if (skill_list.empty())return;
		Now_skill = skill_list[static_cast<size_t>(GetRandomNum(0, static_cast<int>(skill_list.size())-1))].get();//选择一个技能
		if (front.empty())return;
		std::vector<size_t> valid;
		for (size_t i = 0; i < front.size(); ++i)
			if (front[i]) valid.push_back(i);

		if (valid.empty()) return;
		size_t mainIndex = valid[
			static_cast<size_t>(GetRandomNum(0, static_cast<int>(valid.size()) - 1))
		];

		BuildTargetsByMainIndex(front, mainIndex, Now_skill->form.target, skill_targets);
	}
	void Fight_Logic::EnemyExecuting() {
		if (!NowActing)
			return;
		Log(NowActing->Name(), __func__);
		
		if (!Now_skill)
			return;
		
		//auto& front = Actor(PlaceType::FrontCharacter);
		//if (!front.empty()) {
		//	skill_targets.list.push_back({ front[0].get(), TargetRole::Main, 1.0 });
		//}

		Now_skill->Execute(NowActing, skill_targets, *this);
	}
	void Fight_Logic::SetCharacterSkillBySlot(Attribute::SkillSlot slot) {
		auto* c = dynamic_cast<Character_Actor*>(NowActing);
		if (!c) return;

		Now_skill = nullptr;
		for (const auto& skill : c->Skills()) {
			if (skill && skill->form.slot == slot) {
				Now_skill = skill.get();
				break;
			}
		}
		if (!Now_skill) {
			skill_targets.list.clear();
			return;
		}

		auto& enemies = Actor(PlaceType::Enemy);
		std::vector<size_t> valid;
		valid.reserve(enemies.size());
		for (size_t i = 0; i < enemies.size(); ++i)
			if (enemies[i]) valid.push_back(i);

		if (valid.empty()) {
			skill_targets.list.clear();
			return;
		}

		size_t mainIndex = valid[valid.size() / 2];
		BuildTargetsByMainIndex(enemies, mainIndex, Now_skill->form.target, skill_targets);
	}
	void Fight_Logic::CharacterStart_Resolution(Character_Actor* act) {
		if (!act) return;
		CurrentCharacterState = CharacterState::Normal;
		SetCharacterSkillBySlot(Attribute::SkillSlot::Normal);
	}
	void Fight_Logic::CharacterExecuting() {//TODO 还剩下角色的技能执行没有写

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
	void Fight_Logic::DataHandOff(int wintype) {//标志位为胜利还是失败,0是失败，1是胜利
		using StageData::g_Fight_to_FightResult;
		g_Fight_to_FightResult.ifwin = wintype;
		
	}
}