#pragma once
#include<vector>
#include<unordered_map>
#include<array>
#include<map>
#include<string>
#include"AttributeTypes.h"
#include"AttackForm.h"
namespace Attribute {
	
	
	class BasicBuffs {
	public:

		std::unordered_map<ElementType, double>Boost_ElementDamageBoost;

		std::unordered_map<ElementType, double>Resistance_ElementDamageResistance;

		std::unordered_map<AttackType, double>Boost_AttackTypeDamageBoost;//被动给定增伤
	};
	struct BuffConfig {
		std::string id;                 // 唯一标识
		std::string name;
		BasicBuffs add;
		BasicBuffs percent;
		bool isPermanent = false;
		double durationTemplate = 0;    // 模板持续时间
		std::unordered_map<AttackType, double> DamageBoost;//玩法给的增伤
		std::unordered_map<GameAttributeType, std::pair<double, double>> attribute;//玩法给的属性
	};
	struct BuffInstance {
		const BuffConfig* config = nullptr;  // 指向配置
		double remainingActionValue = 0;
		int stacks = 1;//叠层数
	};
	class CharacterBasicAttribute {
	public:
		std::string name = "Unknown";//名字
		int Level = 80;
		ElementType element = ElementType::Physic;
		HeartType BasicHeart = 0;//生命值
		AttackValueType BasicAttack = 0;//攻击力
		DefenseType BasicDefense = 0;//防御力
		SpeedType Speed = 0;//速度
		double CritRate = 0;//暴击率
		double CritDamage = 0;//暴击伤害
		double BreakEffect = 0;//击破特攻
		double HealingBonus = 0;//治疗加成
		int EnergyLimit = 100;//能量上限
		double EnergyRecoverEfficiency = 1;//能量回复效率
		double EffectHitRate = 0;//效果命中
		double EffectResistance = 0;//效果抵抗
		double ElationRate = 0;//欢愉度
		std::vector<AttackForm>SKills;
		std::vector<BuffConfig> defaultBuffs;//配置:备战阶段buff
		//这个是游戏的buff
	};
	class CharacterGameAttribute {
	public:
		int stars = 1;
		int cost = 1;
		SuitPlace suitplace = SuitPlace::Front_Back;
		std::vector<BuffConfig> defaultBuffs;//配置:备战阶段buff
		//这个是币战专属buff
	};
	class CharacterAttribute {
		CharacterBasicAttribute basic_characterattribute;
		CharacterGameAttribute game_characterattribute;
	};
}
