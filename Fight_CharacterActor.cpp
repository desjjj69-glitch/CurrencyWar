#include"Fight_CharacterActor.h"
namespace Fight {
	void Character_Actor::NewActor(const CharacterObject& ch) {
		config = ch;
		name = config.name;
		RealTimeBuffs.clear();
		Energy = 0;
		heart = config.basic_attribute.BasicHeart;
		Speed = config.basic_attribute.Speed;
	}
}