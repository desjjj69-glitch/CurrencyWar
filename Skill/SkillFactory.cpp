// SkillFactory.cpp
#include "SkillFactory.h"
#include "DataDrivenSkill.h"
// 特殊技能以后加在这里：
// #include "Special/AliceUltSkill.h"

namespace Fight {

    std::unique_ptr<BasicSkill> CreateSkill(const Attribute::AttackForm& form) {
        // 特判：按 id 匹配特殊技能
        // if (form.id == "alice_ult")
        //     return std::make_unique<AliceUltSkill>(form);

        // 默认走数据驱动
        return std::make_unique<DataDrivenSkill>(form);
    }
}