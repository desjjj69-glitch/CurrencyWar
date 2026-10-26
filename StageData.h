#pragma once
#include"CharacterObject.h"
#include"EnemyObject.h"
#include<vector>
#include<string>
#include<memory>

namespace StageData {//负责数据交接
	struct GameStart_WaitingForFight {//开始阶段到备战阶段
		std::vector<std::string>Boss_name;
		int difficulty = 0;
		int money_bonus = 0;//环境策略等开局一次性金币加成，进入备战时结算
	};
	struct WaitingForFight_publicData {//公有数据
		size_t* dimension_index;//直接把原始地址给出了
		size_t* stage_index;//之后战斗结算阶段要修改
	};
	struct WaitingForFight_to_Fight {
		const std::vector<std::unique_ptr<BasicObject>>* FrontCharacter=nullptr;//太大而不方便拷贝
		const std::vector<std::unique_ptr<BasicObject>>* BackCharacter = nullptr;
		std::string stagetype;//小数据直接拷贝
		const std::vector<EnemyObject>* Enemys = nullptr;
		int difficulty = 0;
	};
	struct WaitingForFight_to_FightResult {
		int* money = nullptr;
		int* interest_limit = nullptr;
		int* ContinueWinTimes = nullptr;
		int* Blood = nullptr;
		int* ExpNum = nullptr;
		const std::vector<std::vector<std::string>>* stage_type = nullptr;
	};
	struct Fight_to_FightResult {
		bool ifwin;//是否胜利
		double percentage;
	};
	extern WaitingForFight_publicData g_WaitingForFight_publicData;
	extern WaitingForFight_to_Fight g_WaitingForFight_to_Fight;
	extern WaitingForFight_to_FightResult g_WaitingForFight_to_FightResult;
	extern Fight_to_FightResult g_Fight_to_FightResult;
	extern GameStart_WaitingForFight g_GameStart_WaitingForFight;
}