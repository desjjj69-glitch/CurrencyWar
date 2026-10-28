#include"Fight_EnemyActor.h"
#include"Skill/SkillFactory.h"
#include"Skill/ActorSkillRegistry.h"
#include"Fight_Logic.h"
#include"Logger.h"
using Logger::Log;
namespace Fight {
	void Enemy_Actor::NewActor(const std::string& n,int difficulty) {
		name = n;
		config = Object::Enemy_DefaultSetting[name];
		CurrentHeart = config.attribute.heart;
		MaxHeart = CurrentHeart;
		Speed = config.attribute.speed;
		skills.clear();
		for (const auto& skill : config.attribute.Skills) {
			skills.push_back(CreateSkill(skill));
		}

		skillImpl_ = Skill::Registry::Instance().Make(name);
		if (!skillImpl_) skillImpl_ = Skill::Registry::Instance().Make("EnemyDefault");
		if (!skillImpl_) {
			Log("No skill implementation for enemy: " + name, __func__, Logcode::ERROR);
		}
	}
	bool Enemy_Actor::ExecuteSkill(const Attribute::AttackForm& form,
		const SkillTargets& targets,
		BattleContext& ctx) {
		auto* logic = dynamic_cast<Fight_Logic*>(&ctx);
		if (!logic) return false;
		if (!skillImpl_) return false;
		const bool ok = skillImpl_->Execute(*logic, id(), form, targets);
		if (!ok) {
			Log("Enemy skill not implemented: " + name + " / " + form.id,
				__func__, Logcode::ERROR);
		}
		return ok;
	}
}
