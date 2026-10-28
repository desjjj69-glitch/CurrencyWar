#pragma once
#include"AttributeTypes.h"
#include"AttackForm.h"
#include"json.hpp"
#include<string>
namespace Object {
    Attribute::ElementType ParseElement(const std::string& s);

    Attribute::SkillSlot ParseSkillSlot(const std::string& s);

    Attribute::TargetAttackType ParseTargetAttackType(const std::string& s);

    Attribute::AttackType ParseAttackType(const std::string& s);

    //解析一条技能模板（角色和敌人共用）
    Attribute::AttackForm ParseAttackForm(const nlohmann::json& j);
}
