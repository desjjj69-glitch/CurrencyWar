#include"ParseSkill.h"
namespace Object {
    Attribute::ElementType ParseElement(const std::string& s) {
        if (s == "Physic")     return Attribute::ElementType::Physic;
        if (s == "Fire")       return Attribute::ElementType::Fire;
        if (s == "Ice")        return Attribute::ElementType::Ice;
        if (s == "Electro")    return Attribute::ElementType::Electro;
        if (s == "Wind")       return Attribute::ElementType::Wind;
        if (s == "Quantum")    return Attribute::ElementType::Quantum;
        if (s == "Imaginary")  return Attribute::ElementType::Imaginary;
        if (s == "AllElement") return Attribute::ElementType::AllElement;
        return Attribute::ElementType::Physic;
    }

    Attribute::SkillSlot ParseSkillSlot(const std::string& s) {
        if (s == "Normal")    return Attribute::SkillSlot::Normal;
        if (s == "Skill")     return Attribute::SkillSlot::Skill;
        if (s == "Ult")       return Attribute::SkillSlot::Ult;
        if (s == "Talent")    return Attribute::SkillSlot::Talent;
        if (s == "Technique") return Attribute::SkillSlot::Technique;
        if (s == "Assist")    return Attribute::SkillSlot::Assist;
        return Attribute::SkillSlot::Normal;
    }

    Attribute::TargetAttackType ParseTargetAttackType(const std::string& s) {
        if (s == "Single") return Attribute::TargetAttackType::Single;
        if (s == "AoE")    return Attribute::TargetAttackType::AoE;
        if (s == "Bounce") return Attribute::TargetAttackType::Bounce;
        if (s == "Blast")  return Attribute::TargetAttackType::Blast;
        return Attribute::TargetAttackType::Single;
    }

    Attribute::AttackType ParseAttackType(const std::string& s) {
        if (s == "Normal")         return Attribute::AttackType::Normal;
        if (s == "Skill")          return Attribute::AttackType::Skill;
        if (s == "Ult")            return Attribute::AttackType::Ult;
        if (s == "FollowUpAttack") return Attribute::AttackType::FollowUpAttack;
        if (s == "Dot")            return Attribute::AttackType::Dot;
        if (s == "Break")          return Attribute::AttackType::Break;
        if (s == "Elation")        return Attribute::AttackType::Elation;
        if (s == "AllType")        return Attribute::AttackType::AllType;
        return Attribute::AttackType::Normal;
    }
}