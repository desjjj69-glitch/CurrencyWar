#pragma once
#include "../Fight_Logic.h"
#include "../GetRandomNum.h"
#include <algorithm>
#include <vector>

namespace Skill::Helpers {

    inline double MultiplierAt(const Attribute::AttackForm& form, int hit) {
        if (form.multiply.empty())
            return form.atkScale > 0.0 ? form.atkScale : 1.0;
        const size_t index = static_cast<size_t>(hit);
        return index < form.multiply.size() ? form.multiply[index] : form.multiply.back();
    }

    // Every target takes all hits of the skill.
    inline void DamageTargets(Fight::Fight_Logic& logic,
        Fight::BasicActor& caster,
        const Attribute::AttackForm& form,
        const Fight::SkillTargets& targets) {
        for (const auto& target : targets.list) {
            if (!target.actor) continue;
            for (int hit = 0; hit < form.hitCount; ++hit) {
                if (!target.actor->Alive()) break;
                logic.DealDamage(&caster, target.actor, form, MultiplierAt(form, hit));
            }
        }
    }

    // Bounce: each hit picks a random living target.
    inline void DamageBounce(Fight::Fight_Logic& logic,
        Fight::BasicActor& caster,
        const Attribute::AttackForm& form,
        const Fight::SkillTargets& targets,
        double energyPerHit = 0.0) {
        std::vector<Fight::TargetInfo> alive;
        alive.reserve(targets.list.size());
        for (const auto& target : targets.list) {
            if (target.actor && target.actor->Alive()) alive.push_back(target);
        }

        for (int hit = 0; hit < form.hitCount && !alive.empty(); ++hit) {
            const size_t index = static_cast<size_t>(
                GetRandomNum(0, static_cast<int>(alive.size()) - 1));
            if (energyPerHit > 0.0) logic.GainEnergy(&caster, energyPerHit);
            logic.DealDamage(&caster, alive[index].actor, form, MultiplierAt(form, hit));

            alive.erase(
                std::remove_if(alive.begin(), alive.end(),
                    [](const Fight::TargetInfo& target) {
                        return !target.actor || !target.actor->Alive();
                    }),
                alive.end());
        }
    }

}
