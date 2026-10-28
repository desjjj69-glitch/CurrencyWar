#include"Fight_CharacterActor.h"
#include"Skill/SkillFactory.h"
#include"Skill/ActorSkillRegistry.h"
#include"Fight_Logic.h"
#include"Logger.h"
using Logger::Log;
namespace Fight {
	void Character_Actor::NewActor(const CharacterObject& ch) {
		config.name = ch.name;
		config.game_attribute = ch.game_attribute;
		config.basic_attribute = ch.basic_attribute;
		name = config.name;
		RealTimeBuffs.clear();
		Energy = config.basic_attribute.EnergyLimit;
		heart = config.basic_attribute.BasicHeart;
		MaxHeart = heart;
		Speed = config.basic_attribute.Speed;
		skills.clear();
		for (const auto& skill : config.basic_attribute.SKills) {
			skills.push_back(CreateSkill(skill));
		}

		skillImpl_ = Skill::Registry::Instance().Make(name);
		if (!skillImpl_) {
			Log("No skill implementation for character: " + name, __func__, Logcode::ERROR);
		}
		if (skills.empty()) {
			Log("No skills configured for character: " + name, __func__, Logcode::NOTFOUND);
		}
	}
	bool Character_Actor::ExecuteSkill(const Attribute::AttackForm& form,
		const SkillTargets& targets,
		BattleContext& ctx) {
		auto* logic = dynamic_cast<Fight_Logic*>(&ctx);
		if (!logic) return false;
		if (!skillImpl_) return false;
		const bool ok = skillImpl_->Execute(*logic, id(), form, targets);
		if (!ok) {
			Log("Skill not implemented: " + name + " / " + form.id,
				__func__, Logcode::ERROR);
		}
		return ok;
	}
}
