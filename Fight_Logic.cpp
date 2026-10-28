#include"Fight_Logic.h"
#include"StageData.h"
#include"SDL_Functions.h"
#include"Logger.h"
#include"json.hpp"
#include"GetRandomNum.h"
#include<algorithm>
#include<limits>
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
		//已经在行动条上时：既不重复插入，也不能提前推进它的下一次行动值
		for (const auto& entry : ActionList) {
			if (entry.id == act->id()) {
				return;
			}
		}
		act->ActionValue += ActionValueInaRound / act->Speed;
		AddtoActionList({ act->id(),act->ActionValue });
	}
	void Fight_Logic::AddtoExtraAction(ExtraActorEntry act) {
		if (act.id == -1)return;
		// duplicates are allowed: multiple follow-ups or multiple extra turns
		// must each get their own action entry
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
		static bool loaded = false;
		if (loaded)return;
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
		if (js.contains("NowActionValue") && js["NowActionValue"].is_number()) {
			NowActionValue = js["NowActionValue"].get<double>();
		}
		if (js.contains("ActionValueLimit") && js["ActionValueLimit"].is_number()) {
			ActionValueLimit = js["ActionValueLimit"].get<double>();
		}
		if (js.contains("DefaultActionValueLimit") && js["DefaultActionValueLimit"].is_number()) {
			DefaultActionValueLimit = js["DefaultActionValueLimit"].get<double>();
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
		TotalCharacterDamageCount = 0;
		AtFight = FightGlobalState::AtFight;
		GlobalIdIndex = 0;
		SkillPoint = DefaultSkillPoint;
		SkillPointMax = DefaultSkillPointMax;
		NowActionValue = 0;
		ActionValueLimit = DefaultActionValueLimit;
		if (StageData::g_WaitingForFight_to_Fight.stagetype == "Bonus") {
			//Bonus 关卡：行动值上限视为无穷，不会因行动值耗尽失败
			ActionValueLimit = std::numeric_limits<double>::infinity();
		}
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
		//接下来给所有Actor附上唯一编号
		for (auto& vec : actors) {
			for (auto& var : vec) {
				if (!var)continue;
				var->SetId(GlobalIdIndex++);
			}
		}
		for (auto& var : EnemyActorPool) {
			if (!var)continue;
			var->SetId(GlobalIdIndex++);
		}
		for (auto& var : EnemyActorPool) {
			if (!var)continue;
			auto& enemyact = Actor(PlaceType::Enemy);
			Log(var->Name(), __FUNCSIG__);
			if (enemyact.size() < EnemyOnStageNumLimit) {
				enemyact.push_back(std::move(var));
			}
		}
		
	
		
		//Log_Init();
	}
	void Fight_Logic::ShiftCharacterMainTarget(int delta) {
		Log("[Shift] delta=" + std::to_string(delta)
			+ " skill=" + (Now_skill ? Now_skill->form.id : "null"), __func__);
		if (!Now_skill || delta == 0) { Log("[Shift] bail: Now_skill/delta", __func__); return; }

		auto& enemies = Actor(PlaceType::Enemy);
		if (enemies.empty()) return;

		// 1. 找到当前主目标在 enemies 数组里的下标
		int cur = -1;
		if (auto* mainActor = skill_targets.Main()) {
			for (size_t i = 0; i < enemies.size(); ++i) {
				if (enemies[i].get() == mainActor) { cur = static_cast<int>(i); break; }
			}
		}
		Log("[Shift] cur=" + std::to_string(cur), __func__);
		// 2. 没有主目标：从中间向两侧找第一个非空
		if (cur < 0) {
			const int n = static_cast<int>(enemies.size());
			const int mid = n / 2;
			for (int off = 0; off < n; ++off) {
				int l = mid - off, r = mid + off;
				if (l >= 0 && enemies[l]) { cur = l; break; }
				if (r < n && enemies[r]) { cur = r; break; }
			}
			if (cur < 0) return;   // 全空
		}

		// 3. 按方向找下一个非空下标，跳过 nullptr
		const int n = static_cast<int>(enemies.size());
		int next = cur;
		for (int i = 1; i <= n; ++i) {
			int cand = cur + delta * i;
			if (cand < 0 || cand >= n) break;   // 到头了，不动
			if (enemies[cand]) { next = cand; break; }
		}

		// 4. 位置没变就别重建
		Log("[Shift] next=" + std::to_string(next), __func__);
		if (next == cur) { Log("[Shift] no move", __func__); return; }

		BuildTargetsByMainIndex(enemies, static_cast<size_t>(next),
			Now_skill->form.target, skill_targets);
		Log("[Shift] rebuilt mainIndex=" + std::to_string(next), __func__);
	}
	void Fight_Logic::SetCharacterMainTargetByIndex(size_t index) {
		if (!NowActing || NowActing->Type() != ActorType::Character) return;
		if (!Now_skill) return;

		auto& enemies = Actor(PlaceType::Enemy);
		if (index >= enemies.size() || !enemies[index]) return;

		// 已经是主目标就不重建
		if (auto* mainActor = skill_targets.Main()) {
			if (mainActor == enemies[index].get()) return;
		}

		BuildTargetsByMainIndex(enemies, index, Now_skill->form.target, skill_targets);
		Log("[ClickSelect] mainIndex=" + std::to_string(index), __func__);
	}
	void Fight_Logic::QueueCurrentActorResume() {
		// states that interrupt the current action:
		// PlayerStart / EnemyStart / EnemyPreparing
		if (!NowActing) return;
		switch (CurrentState) {
		case StateType::PlayerStart:
			if (NowActing->Type() == ActorType::Character)
				AddtoExtraAction({ NowActing->id(), ExtraPriority::CharacterEnforced });
			break;
		case StateType::EnemyStart:
		case StateType::EnemyPreparing:
			if (NowActing->Type() == ActorType::Enemy)
				AddtoExtraAction({ NowActing->id(), ExtraPriority::EnemyEnforced });
			break;
		default:
			break;
		}
	}
	bool Fight_Logic::TryStartUlt(Character_Actor* p) {
		if (!p) return false;
		if (p->heart <= 0) return false;
		if (p->Energy < p->config.basic_attribute.EnergyLimit) return false;
		if (!p->GetSkillBySlot(Attribute::SkillSlot::Ult)) {
			Log("Ult: no ult skill on " + p->Name(), __func__, Logcode::ERROR);
			return false;
		}

		// states where the ultimate interrupts the current action:
		// Idle / PlayerStart / EnemyStart / EnemyPreparing
		const bool interrupt =
			CurrentState == StateType::Idle ||
			CurrentState == StateType::PlayerStart ||
			CurrentState == StateType::EnemyStart ||
			CurrentState == StateType::EnemyPreparing;

		if (interrupt) {
			QueueCurrentActorResume();// Idle has no active actor, so nothing is queued
		}

		// every ultimate is a separate entry; extra turns must not be merged
		AddtoExtraAction({ p->id(), ExtraPriority::CharacterUlt });
		p->Energy = 0;

		if (interrupt) {
			// start the extra action now; an earlier queued extra action keeps its order
			StartActor();
		}
		return true;
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
			for (size_t i = 0;i < ui_place["Ult_Full"].size();i++) {
				if (PlaceInside(x, y, ui_place["Ult_Full"][i])) {
					if(i>=Actor(PlaceType::FrontCharacter).size())
						continue;
					auto* p = dynamic_cast<Character_Actor*>(Actor(PlaceType::FrontCharacter)[i].get());
					if (!p)return 0;
					if (p->Energy >= p->config.basic_attribute.EnergyLimit&&p->heart>0) {
						return TryStartUlt(p) ? 1 : 0;
					}
				}
			}
			return 0;
			};
		auto CheckIfUltbyKey = [&](const SDL_KeyboardEvent& key) {//键盘版，是否按下了大招并处理
			for (size_t i = 0;i < ui_place["Ult_Full"].size();i++) {
				if (key.scancode == SDL_SCANCODE_1 + i) {
					if (i >= Actor(PlaceType::FrontCharacter).size())
						continue;
					auto* p = dynamic_cast<Character_Actor*>(Actor(PlaceType::FrontCharacter)[i].get());
					if (!p)return 0;
					if (p->Energy >= p->config.basic_attribute.EnergyLimit && p->heart > 0) {
						return TryStartUlt(p) ? 1 : 0;
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
				CheckIfUltbyPos(x, y);
				break;
			case StateType::PlayerSelecting:
				// 选择阶段不打断当前行动，大招只作为额外回合排队
				if (CheckIfUltbyPos(x, y)) return;
				{
					auto& enemies = Actor(PlaceType::Enemy);
					auto& slots = ui_place["Enemy"]; // 用选择框区域当点击判定
					for (size_t i = 0; i < slots.size() && i < enemies.size(); ++i) {
						if (!enemies[i]) continue;
						if (PlaceInside(x, y, slots[i])) {
							SetCharacterMainTargetByIndex(i);
							return;
						}
					}
				}
				switch (CurrentCharacterState) {
				case CharacterState::Normal:
					if (PlaceInside(x, y, ui_place["Skill"][1])) {
						SetCharacterSkillBySlot(Attribute::SkillSlot::Skill);
						CurrentCharacterState = CharacterState::Skill;
					}
					else if (PlaceInside(x, y, ui_place["Normal"][0])) {
						CurrentState = StateType::PlayerExecuting;
						SkillPoint = std::min(++SkillPoint, SkillPointMax);
					}
					break;
				case CharacterState::Skill:
					if (PlaceInside(x, y, ui_place["Normal"][1])) {
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
				CheckIfUltbyPos(x, y);
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
				// 点其他角色的大招：排队，不确认当前大招
				if (CheckIfUltbyPos(x, y)) return;
				//其余位置的鼠标左键确认释放当前大招
				if (Now_skill && NowActing) {
					Now_skill->Execute(NowActing, skill_targets, *this);
				}
				else {
					Log("Ult: no skill/actor to execute", __func__, Logcode::ERROR);
				}
				CurrentState = StateType::PlayerUltExecuting;
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
				CheckIfUltbyKey(key);
				break;
			case StateType::PlayerSelecting:
				// 选择阶段不打断当前行动，大招只作为额外回合排队
				if (CheckIfUltbyKey(key)) return;
				if (key.scancode == SDL_SCANCODE_LEFT||key.scancode==SDL_SCANCODE_A) {
					ShiftCharacterMainTarget(-1);
					return;
				}
				if (key.scancode == SDL_SCANCODE_RIGHT || key.scancode == SDL_SCANCODE_D) {
					ShiftCharacterMainTarget(+1);
					return;
				}
				switch (CurrentCharacterState) {
				case CharacterState::Normal:
					if (key.scancode == SDL_SCANCODE_E) {
						SetCharacterSkillBySlot(Attribute::SkillSlot::Skill);
						CurrentCharacterState = CharacterState::Skill;
					}
					else if (key.scancode == SDL_SCANCODE_Q || key.scancode == SDL_SCANCODE_SPACE) {
						CurrentState = StateType::PlayerExecuting;
						
						SkillPoint = std::min(++SkillPoint, SkillPointMax);
					}
					break;
				case CharacterState::Skill:
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
				CheckIfUltbyKey(key);
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
				if (key.scancode == SDL_SCANCODE_SPACE) {
					//确认释放：增益类大招不需要选敌方目标，直接对已展开的目标结算
					if (Now_skill && NowActing) {
						Now_skill->Execute(NowActing, skill_targets, *this);
					}
					else {
						Log("Ult: no skill/actor to execute", __func__, Logcode::ERROR);
					}
					CurrentState = StateType::PlayerUltExecuting;
				}
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
				if (!act)continue;
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
				if (!act)continue;
				if (act->id() == wact.id) {
					return act.get();
				}
			}
		}
		return nullptr;
	}
	void Fight_Logic::StartActor() {
		auto StateShifter = [&](BasicActor* act) {
			if (!act)return;
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
			auto entry = ExtraAction[0];              // 先复制一份
			NowActing = FindActorFromExtraAction(0);
			RemoveFromExtraList_index(0);

			// 额外行动不删除 ActionList 上的普通回合：
			// CharacterEnforced 既可能是“打断后续接”，也可能是“抽中额外回合”，
			// 后者必须保留角色原本的下一次行动。
			Timer = 0;                                // 关键：切换状态时重置计时器
			switch (entry.Priority) {
			case ExtraPriority::CharacterUlt:
				CurrentState = StateType::PlayerUlt;  // ← 大招走这条
				break;
			case ExtraPriority::CharacterEnforced:
				CurrentState = StateType::PlayerStart;  
				break;
			case ExtraPriority::EnemyEnforced:
				CurrentState = StateType::EnemyStart; // 敌人被推后的行动
				break;
			case ExtraPriority::Normal:
				// 抽中的额外回合：按行动者类型走一次完整回合
				if (!NowActing) {
					CurrentState = StateType::Idle;
					break;
				}
				if (NowActing->Type() == ActorType::Character)
					CurrentState = StateType::PlayerStart;
				else if (NowActing->Type() == ActorType::Enemy)
					CurrentState = StateType::EnemyStart;
				break;
			default:
				// 兜底：按 actor 类型走
				if (!NowActing) {
					CurrentState = StateType::Idle;
					break;
				}
				switch (NowActing->Type()) {
				case ActorType::Character: CurrentState = StateType::PlayerStart; break;
				case ActorType::Enemy:     CurrentState = StateType::EnemyStart; break;
				}
				break;
			}
		}
		else if (!ActionList.empty()) {
			//行动值上限失败判定：当前行动值已达上限，且下一个行动者的行动值也超过上限
			if (NowActionValue >= ActionValueLimit &&
				ActionList.front().ActionValue > ActionValueLimit) {
				bool enemyAlive = false;
				for (auto& e : Actor(PlaceType::Enemy)) {
					if (e) { enemyAlive = true; break; }
				}
				if (enemyAlive) {
					Log("ActionValueLimit reached: now=" + std::to_string(NowActionValue)
						+ " limit=" + std::to_string(ActionValueLimit)
						+ " next=" + std::to_string(ActionList.front().ActionValue),
						__func__);
					DataHandOff(0);
					AtFight = FightGlobalState::EndFight;
					CurrentState = StateType::FightEnd;
					return;
				}
			}
			NowActionValue = ActionList.front().ActionValue;
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
				//大招开始前就选好大招技能和目标（我方全体技能在这里展开）
				SetCharacterSkillBySlot(Attribute::SkillSlot::Ult);
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
				Now_skill = nullptr;
				skill_targets.list.clear();
				CurrentCharacterState = CharacterState::Normal;
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
		//在一次行动之后处理死亡：角色不会真正退场，血量归零立即恢复并扣行动值上限
		auto ReviveDeadCharacters = [&](std::vector<std::unique_ptr<BasicActor>>& vec) {
			for (auto& var : vec) {
				auto* p = dynamic_cast<Character_Actor*>(var.get());
				if (!p)continue;
				if (p->heart <= 0) {
					p->heart = p->MaxHeart * 8 / 10;
					if (p->heart <= 0) p->heart = 1;
					ActionValueLimit -= 35;
					Log("Character death penalty: " + p->Name()
						+ " heart=" + std::to_string(p->heart)
						+ " limit=" + std::to_string(ActionValueLimit), __func__);
				}
			}
		};
		ReviveDeadCharacters(Actor(PlaceType::FrontCharacter));
		ReviveDeadCharacters(Actor(PlaceType::BackCharacter));
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

				if (NowActing == p)NowActing = nullptr;
				var.reset();
				var = GetEnemyFromPool();
				//补位上场的敌人同样要排进行动条：从当前行动值起算一个完整行动间隔
				if (var) {
					var->ActionValue = NowActionValue;
					AddtoActionList(var.get());
					Log("EnemyEnter: " + var->Name()
						+ " action=" + std::to_string(var->ActionValue),
						__func__);
				}
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
				out.list.push_back({ enemies[index].get(),index, role, multiplier });
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
			for (int i = static_cast<int>(mainIndex) - 1; i >= 0; --i) {
				if (i < static_cast<int>(enemies.size()) && enemies[i]) {
					pushTarget(static_cast<size_t>(i), TargetRole::Adjacent);
					break;
				}
			}
			// 向右找第一个非空的
			for (size_t i = mainIndex + 1; i < enemies.size(); ++i) {
				if (enemies[i]) {
					pushTarget(i, TargetRole::Adjacent);
					break;
				}
			}
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
		for (auto& var : Actor(PlaceType::FrontCharacter)) {
			if (!var)continue;
			auto* p = dynamic_cast<Character_Actor*>(var.get());
			if (!p)continue;
			Log("HeartCharacter:" + std::to_string(p->heart), __func__);
		}

	}
	void Fight_Logic::SetCharacterSkillBySlot(Attribute::SkillSlot slot) {
		auto* c = dynamic_cast<Character_Actor*>(NowActing);
		if (!c) {
			Log("SetCharacterSkillBySlot: NowActing is not Character_Actor", __func__, Logcode::ERROR);
			return;
		}

		Now_skill = nullptr;
		for (const auto& skill : c->Skills()) {
			if (!skill) continue;
			if (skill->form.slot == slot) {
				Now_skill = skill.get();
				break;
			}
		}

		if (!Now_skill) {
			Log("SetCharacterSkillBySlot: NO skill for slot="
				+ std::to_string(static_cast<int>(slot))
				+ " on " + c->Name()
				+ " (total skills=" + std::to_string(c->Skills().size()) + ")",
				__func__, Logcode::ERROR);
			skill_targets.list.clear();
			return;
		}

		//我方全体技能（如全队加速）：目标从场上角色里展开，不选敌方
		if (Now_skill->form.target == Attribute::TargetAttackType::AllAlly) {
			skill_targets.list.clear();
			size_t index = 0;
			auto PushAllies = [&](PlaceType place) {
				for (auto& var : Actor(place)) {
					if (!var || !var->Alive()) continue;
					if (var->Type() != ActorType::Character) continue;
					skill_targets.list.push_back({ var.get(), index++, TargetRole::Ally, 1.0 });
				}
				};
			PushAllies(PlaceType::FrontCharacter);
			PushAllies(PlaceType::BackCharacter);
			Log("SetCharacterSkillBySlot: OK skill=" + Now_skill->form.id
				+ " slot=" + std::to_string(static_cast<int>(slot))
				+ " target_type=AllAlly list_size=" + std::to_string(skill_targets.list.size()),
				__func__);
			return;
		}

		auto& enemies = Actor(PlaceType::Enemy);
		std::vector<size_t> valid;
		valid.reserve(enemies.size());
		for (size_t i = 0; i < enemies.size(); ++i)
			if (enemies[i]) valid.push_back(i);

		if (valid.empty()) {
			Log("SetCharacterSkillBySlot: no valid enemy", __func__, Logcode::ERROR);
			skill_targets.list.clear();
			return;
		}

		size_t mainIndex = valid[valid.size() / 2];
		BuildTargetsByMainIndex(enemies, mainIndex, Now_skill->form.target, skill_targets);

		Log("SetCharacterSkillBySlot: OK skill=" + Now_skill->form.id
			+ " slot=" + std::to_string(static_cast<int>(slot))
			+ " target_type=" + std::to_string(static_cast<int>(Now_skill->form.target))
			+ " list_size=" + std::to_string(skill_targets.list.size()),
			__func__);
	}
	void Fight_Logic::CharacterStart_Resolution(Character_Actor* act) {
		if (!act) return;
		CurrentCharacterState = CharacterState::Normal;
		SetCharacterSkillBySlot(Attribute::SkillSlot::Normal);
	}
	void Fight_Logic::CharacterExecuting() {
		if (!NowActing)return;
		Log(NowActing->Name(), __func__);
		if (!Now_skill)return;
		Now_skill->Execute(NowActing, skill_targets, *this);
		for (auto& var : Actor(PlaceType::Enemy)) {
			if (!var)continue;
			auto* p = dynamic_cast<Enemy_Actor*>(var.get());
			if (!p)continue;
			Log("HeartEnemy:" + std::to_string(p->CurrentHeart), __func__);
		}
	}
	void Fight_Logic::DealDamage(BasicActor* caster, BasicActor* target,
		const Attribute::AttackForm& form,
		double multiplier) {//单段伤害结算：段数和每段倍率由技能在 Execute 里展开
		if (!caster || !target)
			return;
		Attribute::AttackValueType damage = CalculateDamage(caster, target, form, multiplier);
		switch (target->Type()) {
		case ActorType::Character:
		{
			auto* c = dynamic_cast<Character_Actor*>(target);
			c->heart -= damage;
		}
		break;
		case ActorType::Enemy:
		{
			auto* e = static_cast<Enemy_Actor*>(target);
			e->CurrentHeart -= damage;
			TotalCharacterDamageCount += damage;
		}
		break;
		}
	}
	void Fight_Logic::GainEnergy(BasicActor* who, double amount) {
		if (who->Type() == ActorType::Character) {
			auto* p = dynamic_cast<Character_Actor*>(who);
			p->Energy += static_cast<int>(amount*p->config.basic_attribute.EnergyRecoverEfficiency);
			
		}
	}
	void Fight_Logic::ApplySpeedBoost(BasicActor* target, double percent) {
		if (!target || percent <= 0.0) return;
		const double oldSpeed = target->Speed;
		if (oldSpeed <= 0.0) return;
		const double newSpeed = oldSpeed * (1.0 + percent);
		if (newSpeed <= oldSpeed) return;
		target->Speed = newSpeed;

		//行动条上的剩余路程按速度比例压缩，然后重排，让加速立刻体现在行动条上
		if (target->ActionValue > NowActionValue) {
			const double remaining = target->ActionValue - NowActionValue;
			target->ActionValue = NowActionValue + remaining * (oldSpeed / newSpeed);
			for (auto& entry : ActionList) {
				if (entry.id == target->id()) {
					entry.ActionValue = target->ActionValue;
				}
			}
			std::sort(ActionList.begin(), ActionList.end());
		}
		Log("SpeedBoost: " + target->Name()
			+ " speed=" + std::to_string(oldSpeed) + "->" + std::to_string(newSpeed)
			+ " action=" + std::to_string(target->ActionValue), __func__);
	}
	void Fight_Logic::ApplyBuff(BasicActor* target, const std::string& buffId) {//上buff也没写

	}
	void Fight_Logic::DataHandOff(int wintype) {//标志位为胜利还是失败,0是失败，1是胜利
		using StageData::g_Fight_to_FightResult;
		g_Fight_to_FightResult.ifwin = wintype;
		
	}
}