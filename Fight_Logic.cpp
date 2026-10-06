#include"Fight_Logic.h"
#include"StageData.h"
#include"SDL_Functions.h"
#include"Logger.h"
#include"json.hpp"
#include<algorithm>
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
	void Fight_Logic::LoadinJson(const std::filesystem::path& gp) {//目前没有从json读入的内容，全是内部数据交接

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
		for (auto& event : EventQueue) {
			switch (event.type) {
			case SDL_EVENT_KEY_DOWN:
				switch (event.key.scancode) {
				case SDL_SCANCODE_ESCAPE:
					Log(SDL_GetKeyName(event.key.key), __func__);
					AtFight = FightGlobalState::EndFight;
					break;
				default:
					Log(SDL_GetKeyName(event.key.key), __func__);
					break;
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
}