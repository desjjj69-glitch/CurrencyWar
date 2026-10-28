#pragma once
#include"AttributeTypes.h"
#include<string>
#include<vector>
namespace Attribute {
	struct AttackForm {
		std::string id;
		std::string name;

		SkillSlot slot = SkillSlot::Normal;//角色技能
		TargetAttackType target = TargetAttackType::Single;//伤害目标

		std::vector<AttackType> types;//伤害类别

		ElementType element = ElementType::Physic;//元素属性

		double atkScale = 0.0;
		double hpScale = 0.0;
		double defScale = 0.0;
		std::vector<double>multiply;//每段伤害的倍率
		int hitCount = 1;//伤害段数
		double breakValue = 0.0;//削韧值
		double energyGain = 0.0;//能量获取
		double speedBoostPercent = 0.0;//行动条加速百分比，0.15 表示 +15%

		//是否包含伤害：全为 0 时表示纯增益/减益技能，不进入伤害结算
		bool HasDamage()const {
			return atkScale > 0.0 || hpScale > 0.0 || defScale > 0.0;
		}
	};
}