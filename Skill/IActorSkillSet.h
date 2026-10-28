#pragma once
#include "../AttackForm.h"
#include "../SkillTargets.h"

namespace Fight { class Fight_Logic; }

namespace Skill {

    // One implementation object per actor (character or enemy).
    // Logic only selects the skill + targets and sends the cast signal;
    // the concrete actor implementation performs the effect here.
    class IActorSkillSet {
    public:
        virtual ~IActorSkillSet() = default;

        bool Execute(Fight::Fight_Logic& logic,
            int casterId,
            const Attribute::AttackForm& form,
            const Fight::SkillTargets& targets) {
            switch (form.slot) {
            case Attribute::SkillSlot::Normal:
                return Normal(logic, casterId, form, targets);
            case Attribute::SkillSlot::Skill:
                return Skill(logic, casterId, form, targets);
            case Attribute::SkillSlot::Ult:
                return Ultimate(logic, casterId, form, targets);
            case Attribute::SkillSlot::Talent:
                return Talent(logic, casterId, form, targets);
            default:
                return false;
            }
        }

    protected:
        virtual bool Normal(Fight::Fight_Logic&, int,
            const Attribute::AttackForm&, const Fight::SkillTargets&) { return false; }
        virtual bool Skill(Fight::Fight_Logic&, int,
            const Attribute::AttackForm&, const Fight::SkillTargets&) { return false; }
        virtual bool Ultimate(Fight::Fight_Logic&, int,
            const Attribute::AttackForm&, const Fight::SkillTargets&) { return false; }
        virtual bool Talent(Fight::Fight_Logic&, int,
            const Attribute::AttackForm&, const Fight::SkillTargets&) { return false; }
    };

}
