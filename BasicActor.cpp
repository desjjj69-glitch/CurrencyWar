#include"BasicActor.h"
namespace Fight {
	BasicSkill* BasicActor::GetSkillById(const std::string& skillId) {
		for (auto& s : skills)
			if (s && s->form.id == skillId) return s.get();
		return nullptr;
	}

	BasicSkill* BasicActor::GetSkillByIndex(size_t i) {
		return i < skills.size() ? skills[i].get() : nullptr;
	}
	const std::vector<std::unique_ptr<BasicSkill>>& BasicActor::Skills() const {
		return skills;
	}
}