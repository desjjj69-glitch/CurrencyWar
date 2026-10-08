#include"Fight_CharacterActor.h"
#include"Skill/SkillFactory.h"
namespace Fight {
	void Character_Actor::NewActor(const CharacterObject& ch) {
		config = ch;
		name = config.name;
		RealTimeBuffs.clear();
		Energy = 0;
		heart = config.basic_attribute.BasicHeart;
		Speed = config.basic_attribute.Speed;
		skills.clear();
		for (const auto& skill : config.basic_attribute.SKills) {
			skills.push_back(CreateSkill(skill));
		}
		if (skills.empty()) {//TODO 此处仅为快速测试，之后要在json加入角色技能
			Attribute::AttackForm fallback;
			fallback.id = name + "_normal_fallback";
			fallback.name = "Normal";
			fallback.slot = Attribute::SkillSlot::Normal;
			fallback.target = Attribute::TargetAttackType::Single;
			fallback.element = Attribute::ElementType::Physic;
			fallback.types.push_back(Attribute::AttackType::Normal);
			fallback.atkScale = 1.0;
			fallback.hitCount = 1;
			skills.push_back(CreateSkill(fallback));
		}
	}
}