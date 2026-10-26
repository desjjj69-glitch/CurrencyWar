#pragma once
#include"CharacterObject.h"
#include"EnemyObject.h"
#include"WaitingForFight_Shop.h"
#include"MineralType.h"
#include"Equipment/EquipmentType.h"
#include"SDL_Functions.h"
#include"Strategy/StrategyBus.h"
#include<memory>
#include<string>
#include<unordered_map>
#include<map>
#include<optional>
#include<filesystem>
#include<vector>
#include<array>
class Mineral;//定义见 Mineral.h
class BasicEquipment;//定义见 Equipment/BasicEquipment.h
namespace WaitingForFight {
	//晶矿内容类型
	enum class MineralContentType {
		Character = 0,
		Equipment,
		Money,
		Exp
	};
	//生成晶矿时对“一类内容”的请求；未填的字段按品质默认规则/随机补全
	struct MineralContentSpec {
		MineralContentType type = MineralContentType::Character;
		std::optional<int> cost;                        // Character 费用；不填按品质默认
		std::optional<int> star;                        // Character 星级；不填按品质默认
		std::optional<EquipmentType> equipmentQuality;  // Equipment 品质；不填按品质默认
		std::optional<std::string> name;                // Character/Equipment 具体名字；不填随机
		int amount = 0;                                 // Money/Exp 数量
		int count = 1;                                  // Character/Equipment 数量
	};
	class Logic:public Strategy::IGameContext {//保存所有备战阶段的资源
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
		bool LeftButtonDown = false;//左键是否处于按下状态（用于拖动扫矿）
		
		enum class OriginPlaceType {
			FrontBattleField=0,
			BackBattleField,
			Bench,
			ShopCharacter,
			ConsumableEquipment,
			Equipment,
			Mineral,
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

		//各品质晶矿的图片尺寸（先写死，以后从json读入）
		std::map<MineralType, SDL_FPoint> MineralSize;

		//装备摆放规则（从 WaitingForFight_ObjectPlace.json 的 EquipmentLayout 读入）
		SDL_FPoint EquipmentSize{ 100.f, 100.f };
		SDL_FPoint ConsumableEquipmentOrigin{ 2419.f, 190.f };//Consumable 从右往左排，单行
		SDL_FPoint EquipmentOrigin{ 2419.f, 290.f };//其它装备：一列 6 个，放满后往左换列
		int EquipmentColumnLimit = 6;

		std::vector<std::vector<std::string>> stage;//关卡类型
		size_t dimension_index = 0;//所在位面（0开头)
		size_t stage_index = 0;//所在关卡
		
		int Blood = 80;

		int ContinueWinTimes = 0;
		int last_continue_win_times = -1;//上次通知策略的连胜数
		int last_dimension = -1;//上次通知策略的位面

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

		//按“品质 + 内容请求”生成一个晶矿；contents 为空表示内容未指定，打开时按品质随机。
		//例：State.GetMineral(MineralType::Grey);                                 // 全随机
		//    State.GetMineral(MineralType::Grey, { {.type=MineralContentType::Character} });      // 随机1费角色
		//    State.GetMineral(MineralType::Blue, { {.type=MineralContentType::Character, .cost=2, .star=3} });
		//    State.GetMineral(MineralType::Grey, { {.type=MineralContentType::Money, .amount=1} });
		std::unique_ptr<Mineral> GetMineral(MineralType quality,
			const std::vector<MineralContentSpec>& contents = {});

		//把一件装备放进 ConsumableEquipment / Equipment 槽位，并按布局规则追加位置
		bool AddEquipment(std::unique_ptr<BasicEquipment> equipment);

		//给策略使用的接口
		void AddMoney(int amount) { MoneyNum += amount; }
		void AddMineralMoney(int money);//生成一个只有 money 的晶矿并放进 Mineral 槽位
		void AddMineralCharacter(int cost);//生成一个随机指定费用角色的晶矿并放进 Mineral 槽位
		bool AddEquipmentByName(const std::string& name);//按名字创建装备并放进装备槽位

		
		void Click();
		void LongPress();
		
		int PutDown(float,float);
		int PickUp();
		void PutBack();
		

		void HandleEvent(const SDL_Event&);//所有事件

		void EventLogic(SDL_Event&);//处理所有逻辑事务

		
	private:
		bool PlaceInside(float, float, const SDL_FRect&);
		bool HandleMineralPress(float, float);//点中晶矿返回 true：可开则开，备战席不够则吃掉点击
		void Init_Minerals();//开局固定的 2 灰 + 1 蓝放进 Mineral 槽位（只在 Loadin 时调用一次）
		bool AddMineral(std::unique_ptr<Mineral> mineral);//按随机位置追加一个晶矿
		SDL_FRect EquipmentRect(OriginPlaceType type, size_t index)const;//按布局算第 index 件装备的位置
		void RelayoutEquipment(OriginPlaceType type);//装备被取走后重排剩余装备位置
		CharacterObject* CharacterAt(float x, float y);//取指定位置的场上/备战席角色
		void ReturnCharacterEquipments(CharacterObject* character);//角色离开时把装备放回装备槽位
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
	public:
		Strategy::StageKind Kind()const { return Strategy::StageKind::WaitingForFight; }
		
	};
	extern Logic State;
}