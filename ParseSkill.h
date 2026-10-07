#pragma once
#include"AttributeTypes.h"
#include<string>
namespace Object {
    Attribute::ElementType ParseElement(const std::string& s);

    Attribute::SkillSlot ParseSkillSlot(const std::string& s);

    Attribute::TargetAttackType ParseTargetAttackType(const std::string& s);

    Attribute::AttackType ParseAttackType(const std::string& s);
}