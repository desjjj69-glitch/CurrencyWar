#pragma once
#include"AttributeTypes.h"
#include"AttackForm.h"
#include"SkillTargets.h"
namespace Fight {
	class BasicActor;
	class BattleContext;
	class BasicSkill {
	public:
		virtual ~BasicSkill() = default;
		Attribute::AttackForm form;
		virtual void Execute(BasicActor* user, const SkillTargets& targets ,BattleContext& ctx) = 0;
	};
}