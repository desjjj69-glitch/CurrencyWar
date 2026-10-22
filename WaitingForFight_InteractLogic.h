#pragma once
#include"CharacterObject.h"
#include"EnemyObject.h"
#include"WaitingForFight_Shop.h"
#include"SDL_Functions.h"
#include<memory>
#include<string>
#include<unordered_map>
#include<filesystem>
#include<vector>
#include<array>
namespace WaitingForFight {
	class Logic {//保存所有备战阶段的资源
	public:
		enum class RealTimeMode {
			Normal,
			Shop,
			DetailInfo
		};
		RealTimeMode Rmode = RealTimeMode::Normal;
		bool AtWaitingForFight = false;
		enum class MouseMode {
			Idle,
			Pressed,
			LongPressed,
			Dragging
		};
		MouseMode Mmode = MouseMode::Idle;
		
		enum class OriginPlaceType {
			FrontBattleField=0,
			BackBattleField,
			Bench,
			ShopCharacter,
			ConsumableEquipment,
			Equipment,
			Count
		};
		
		std::unique_ptr<BasicObject> holding;
		OriginPlaceType holdoriginplace = OriginPlaceType::FrontBattleField;
		size_t OriginIndex = 0;
		SDL_FRect OriginPlace = {};
		SDL_FRect Offset = {};
		
		Uint64 PressStartTime = 0;
		Uint64 PressEndTime = 0;
		float PressStartX = 0;
		float PressStartY = 0;
		float MotionX = 0;
		float MotionY = 0;
		static constexpr Uint64 LONG_PRESS_MS = 350;
		static constexpr float CAN_DRAG_DISTANCE = 5;
		


	private:
		static constexpr size_t Idx(OriginPlaceType t) noexcept { return static_cast<size_t>(t); }
		static constexpr size_t kPlaceCount = static_cast<size_t>(OriginPlaceType::Count);
	
		std::array<std::vector<std::unique_ptr<BasicObject>>, kPlaceCount> zone_;//各个槽位的物品
		std::array<std::vector<SDL_FRect>, kPlaceCount> objectplace_;//各个槽位的位置
		//主要对象：角色/箱子/聘书
		//Minerals
		//Equipments
		//Bonds
		//Strategies
	public:
		std::vector<std::unique_ptr<BasicObject>>& Zone(OriginPlaceType t) {
			return zone_[Idx(t)];
		}
		const std::vector<std::unique_ptr<BasicObject>>& Zone(OriginPlaceType t) const {
			return zone_[Idx(t)];
		}
		std::vector<SDL_FRect>& ObjectPlace(OriginPlaceType t) {
			return objectplace_[Idx(t)];
		}
		const std::vector<SDL_FRect>& ObjectPlace(OriginPlaceType t)const {
			return objectplace_[Idx(t)];
		}
		std::unordered_map<std::string, SDL_FRect> buttonplace;//ui的各个按钮的位置，和槽位不同

		std::vector<std::vector<std::string>> stage;//关卡类型
		size_t dimension_index = 0;//所在位面（0开头)
		size_t stage_index = 0;//所在关卡
		
		int Blood = 80;

		int ContinueWinTimes = 0;

		Shop shop;

		int MoneyNum = 0;
		int MaxInterest = 5;

		int CharacterOnBattleFieldLimit = 3;//出战席人数限制
		int CharacterOnBenchLimit = 9;//备战席人数限制

		int Difficulty = 137;//关卡难度

		std::vector<std::string>boss_name;//从开局读入
		std::vector<EnemyObject> Boss;//开局先生成3个Boss
		std::vector<EnemyObject> Enemys;//每个关卡单独生成怪，在进入关卡的这个阶段就生成好
		void LoadinJson(const std::filesystem::path&);
		void Init();
		Logic();
		~Logic() = default;

		
		void Click();
		void LongPress();
		
		int PutDown(float,float);
		int PickUp();
		void PutBack();
		

		void HandleEvent(const SDL_Event&);//所有事件

		void EventLogic(SDL_Event&);//处理所有逻辑事务
	private:
		bool PlaceInside(float, float, const SDL_FRect&);
		bool DragDisTest(const SDL_MouseMotionEvent&);
		int CountResouceNum(OriginPlaceType t) {
			int num = 0;
			for (int i = 0;i < Zone(t).size();i++) {
				if (Zone(t)[i] != nullptr) {
					num++;
				}
			}
			return num;
		}
		void DataHandOff();
		bool StarUp();
		void Init_Enemys();
		void Init_Boss();
	};
	extern Logic State;
}