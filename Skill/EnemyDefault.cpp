#include "EnemyDefault.h"
#include "ActorSkillRegistry.h"
#include "ActorSkillHelpers.h"
#include "../Fight_Logic.h"
#include "../BasicActor.h"
#include "../SkillTargets.h"

REGISTER_ACTOR_SKILLS("EnemyDefault", Skill::Impl::EnemyDefaultSkillSet);

namespace Skill::Impl {

    namespace {
        bool Cast(Fight::Fight_Logic& logic, int casterId,
            const Attribute::AttackForm& form, const Fight::SkillTargets& targets) {
            auto* self = logic.FindActorById(casterId);
            if (!self) return false;
            if (!form.HasDamage()) return true;

            if (form.target == Attribute::TargetAttackType::Bounce) {
                Helpers::DamageBounce(logic, *self, form, targets);
            }
            else {
                Helpers::DamageTargets(logic, *self, form, targets);
            }
            return true;
        }
    }

    bool EnemyDefaultSkillSet::Normal(Fight::Fight_Logic& logic, int casterId,
        const Attribute::AttackForm& form, const Fight::SkillTargets& targets) {
        return Cast(logic, casterId, form, targets);
    }

    bool EnemyDefaultSkillSet::Skill(Fight::Fight_Logic& logic, int casterId,
        const Attribute::AttackForm& form, const Fight::SkillTargets& targets) {
        return Cast(logic, casterId, form, targets);
    }

    bool EnemyDefaultSkillSet::Ultimate(Fight::Fight_Logic& logic, int casterId,
        const Attribute::AttackForm& form, const Fight::SkillTargets& targets) {
        return Cast(logic, casterId, form, targets);
    }

}
