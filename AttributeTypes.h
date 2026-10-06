#pragma once
#include<cstdint>
namespace Attribute {
	using HeartType = std::int64_t;
	using AttackValueType = std::int64_t;
	using SpeedType = double;
	using DefenseType = int;
	enum class ElementType {
		Physic,//物理
		Fire,//火
		Ice,//冰
		Electro,//雷
		Wind,//风
		Quantum,//量子
		Imaginary,//虚数
		AllElement,
		Count
	};
	enum class AttackType {//用来计算增伤
		Normal,//普攻
		Skill,//战技
		Ult,//大招
		FollowUpAttack,//追加攻击
		Dot,//持续伤害
		Break,//击破
		Elation,//欢愉
		AllType,//全属性增伤
		Count
	};
	enum class SuitPlace {
		Front,
		Back,
		Front_Back
	};
	enum class GameAttributeType {
		FrontStrength,
		BackStrength,
		DamageReduction,
		LuckyHitRate,
		LuckyHitDamage,
		Speed_Boost,
		HealingStrength,
		ArmorStrength
	};
	enum class SkillSlot {//角色的技能槽位，固定
		Normal,
		Skill,
		Ult,
		Talent,
		Technique,
		Count
	};
	enum class TargetAttackType {//伤害目标
		Single,
		AoE,
		Bounce,
		Blast
	};
}