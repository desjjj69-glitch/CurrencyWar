#pragma once
#include"Fight_ActorType.h"
#include"Fight_CharacterActor.h"
#include"Fight_EnemyActor.h"
#include"BasicActor.h"
#include"SDL_Functions.h"
#include<unordered_map>
#include<memory>
#include<filesystem>
#include<vector>
#include<array>
namespace Fight {
	class Fight_Logic {
	public:
		enum class PlaceType {
			FrontCharacter,
			BackCharacter,
			Summon,
			Aha,
			Enemy,
			Equipment,
			Count
		};
		std::vector<std::unique_ptr<BasicActor>>& Actor(PlaceType t) {
			return actors[static_cast<size_t>(t)];
		}
		std::vector<ActorList>ActionList;
		std::vector<ActorList>ExtraAction;

		void LoadinJson(const std::filesystem::path&);
		void Init();
		BasicActor* FindActorFromActionList(size_t);
		BasicActor* FindActorFromExtraAction(size_t);
		enum class FightGlobalState {
			AtFight,
			Pause,
			EndFight
		};
		FightGlobalState AtFight = FightGlobalState::EndFight;
		std::vector<SDL_Event> EventQueue;
		void PollEvents();
		void DispatchEvent();
		enum class StateType {//行动状态
			Idle,               // 空闲：在等玩家输入 / 在等行动条的下一项
			PlayerStart,//角色回合开始，结算角色身上挂的dot伤害
			PlayerSelecting,    // 玩家正在选技能/选目标
			PlayerExecuting,    // 玩家动作的动画/结算播放中
			EnemyStart,     // 敌人结算dot伤害
			EnemyPreparing,//敌人抬手，选择目标准备攻击
			EnemyExecuting,     // 敌人动作的动画/结算播放中
			InsertAction,       // 玩家插入的额外回合（大招）执行中
			TurnEnd,            // 收尾：更新行动条、切换到下一个
			FightEnd//战斗结束，此时
		};
		StateType CurrentState = StateType::Idle;
		void Tick(Uint64);//处理逻辑帧
		Uint64 Timer = 0;
		BasicActor* NowActing=nullptr;
	private:
		int GlobalIdIndex = 0;
		
		static constexpr size_t ActorCount = static_cast<size_t>(PlaceType::Count);
		std::array<std::vector<std::unique_ptr<BasicActor>>, ActorCount >actors;
		void BuildActionList();
		void AddtoActionList(ActorList);
		void RemoveFromActionList_id(int);
		void RemoveFromActionList_index(size_t);
		void RemoveFromExtraList_id(int);
		void RemoveFromExtraList_index(size_t);
		void StartActor();
		void Log_Init();
	};
	extern Fight_Logic State;
}