#include "Asta.h"
#include "ActorSkillRegistry.h"
#include "ActorSkillHelpers.h"
#include "../Fight_Logic.h"
#include "../Fight_CharacterActor.h"
#include "../SkillTargets.h"

REGISTER_ACTOR_SKILLS("Asta", Skill::Impl::AstaSkillSet);

namespace Skill::Impl {

    namespace {
        Fight::Character_Actor* FindCaster(Fight::Fight_Logic& logic, int casterId) {
            return dynamic_cast<Fight::Character_Actor*>(logic.FindActorById(casterId));
        }
    }

    bool AstaSkillSet::Normal(Fight::Fight_Logic& logic, int casterId,
        const Attribute::AttackForm& form, const Fight::SkillTargets& targets) {
        auto* self = FindCaster(logic, casterId);
        if (!self) return false;
        if (form.energyGain > 0.0) logic.GainEnergy(self, form.energyGain);
        Helpers::DamageTargets(logic, *self, form, targets);
        return true;
    }

    bool AstaSkillSet::Skill(Fight::Fight_Logic& logic, int casterId,
        const Attribute::AttackForm& form, const Fight::SkillTargets& targets) {
        auto* self = FindCaster(logic, casterId);
        if (!self) return false;
        if (form.energyGain > 0.0) logic.GainEnergy(self, form.energyGain);
        Helpers::DamageBounce(logic, *self, form, targets);
        return true;
    }

    bool AstaSkillSet::Ultimate(Fight::Fight_Logic& logic, int casterId,
        const Attribute::AttackForm& form, const Fight::SkillTargets& targets) {
        auto* self = FindCaster(logic, casterId);
        if (!self) return false;
        if (form.energyGain > 0.0) logic.GainEnergy(self, form.energyGain);

        if (form.HasDamage()) {
            Helpers::DamageTargets(logic, *self, form, targets);
        }
        if (form.speedBoostPercent > 0.0) {
            for (const auto& target : targets.list) {
                if (!target.actor || !target.actor->Alive()) continue;
                logic.ApplySpeedBoost(target.actor, form.speedBoostPercent);
            }
        }
        return true;
    }

}
