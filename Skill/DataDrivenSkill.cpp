#include"DataDrivenSkill.h"
#include "../BasicActor.h"
#include "../BattleContext.h"

namespace Fight {

    void DataDrivenSkill::Execute(BasicActor* user,
        const SkillTargets& targets,
        BattleContext& ctx) {
        if (form.energyGain > 0)
            ctx.GainEnergy(user, form.energyGain);

        for (const auto& t : targets.list) {
            if (!t.actor) continue;
            ctx.DealDamage(user, t.actor, form, t.damageMultiplier);
        }
    }
}