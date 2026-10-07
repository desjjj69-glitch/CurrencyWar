#pragma once
#include"../BasicSkill.h"
namespace Fight {
    class DataDrivenSkill : public BasicSkill {
    public:
        DataDrivenSkill() = default;
        explicit DataDrivenSkill(const Attribute::AttackForm& f) { form = f; }

        void Execute(BasicActor* user,
            const SkillTargets& targets,
            BattleContext& ctx) override;
    };
}