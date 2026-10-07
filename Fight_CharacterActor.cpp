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
	}
}