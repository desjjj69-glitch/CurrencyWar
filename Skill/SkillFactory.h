// SkillFactory.h
#pragma once
#include "../BasicSkill.h"
#include <memory>

namespace Fight {
    std::unique_ptr<BasicSkill> CreateSkill(const Attribute::AttackForm&);
}
