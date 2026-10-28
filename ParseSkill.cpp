#include"ParseSkill.h"
#include<algorithm>
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
        if (s == "AllAlly" || s == "AllyAll" || s == "Ally")
            return Attribute::TargetAttackType::AllAlly;
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

    Attribute::AttackForm ParseAttackForm(const nlohmann::json& j) {
        Attribute::AttackForm form;
        if (!j.is_object()) {
            form.multiply.push_back(0.0);
            return form;
        }

        form.id = j.value("id", std::string(""));
        form.name = j.value("name", std::string(""));
        form.slot = ParseSkillSlot(j.value("slot", std::string("Normal")));
        form.target = ParseTargetAttackType(j.value("target", std::string("Single")));
        form.element = ParseElement(j.value("element", std::string("Physic")));

        if (j.contains("types") && j["types"].is_array()) {
            for (const auto& t : j["types"]) {
                if (t.is_string()) {
                    form.types.push_back(ParseAttackType(t.get<std::string>()));
                }
            }
        }
        if (form.types.empty()) {
            form.types.push_back(Attribute::AttackType::Normal);
        }

        form.atkScale = j.value("atkScale", 0.0);
        form.hpScale = j.value("hpScale", 0.0);
        form.defScale = j.value("defScale", 0.0);
        form.hitCount = std::max(1, j.value("hitCount", 1));
        form.breakValue = j.value("breakValue", 0.0);
        form.energyGain = j.value("energyGain", 0.0);
        form.speedBoostPercent = j.value("speedBoostPercent", j.value("speedBoost", 0.0));

        //每段倍率：优先读 multiply 数组；没写则把 atkScale 当作每段倍率
        if (j.contains("multiply") && j["multiply"].is_array()) {
            for (const auto& m : j["multiply"]) {
                if (m.is_number()) {
                    form.multiply.push_back(m.get<double>());
                }
            }
        }
        if (form.multiply.empty()) {
            const double perHit = form.atkScale != 0.0 ? form.atkScale : 1.0;
            form.multiply.assign(static_cast<size_t>(form.hitCount), perHit);
        }
        //段数和倍率数组对齐，避免结算时越界
        if (static_cast<int>(form.multiply.size()) < form.hitCount) {
            const double last = form.multiply.empty() ? 1.0 : form.multiply.back();
            form.multiply.resize(static_cast<size_t>(form.hitCount), last);
        }
        else if (static_cast<int>(form.multiply.size()) > form.hitCount) {
            form.multiply.resize(static_cast<size_t>(form.hitCount));
        }

        return form;
    }
}
