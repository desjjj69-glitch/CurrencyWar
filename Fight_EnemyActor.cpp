#include"Fight_EnemyActor.h"
namespace Fight {
	void Enemy_Actor::NewActor(const std::string& n,int difficulty) {
		name = n;
		config = Object::Enemy_DefaultSetting[name];
		CurrentHeart = config.attribute.heart;
		Speed = config.attribute.speed;
	}
}