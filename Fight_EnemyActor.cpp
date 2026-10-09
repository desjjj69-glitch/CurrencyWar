#include"Fight_EnemyActor.h"
#include"Skill/SkillFactory.h"
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
	}
}