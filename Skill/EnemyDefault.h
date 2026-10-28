#pragma once
#include "IActorSkillSet.h"

namespace Skill::Impl {

    // Generic enemy skill implementation. Specific enemies can register their
    // own implementation under their JSON name; this one is the fallback.
    class EnemyDefaultSkillSet : public IActorSkillSet {
    protected:
        bool Normal(Fight::Fight_Logic&, int,
            const Attribute::AttackForm&, const Fight::SkillTargets&) override;
        bool Skill(Fight::Fight_Logic&, int,
            const Attribute::AttackForm&, const Fight::SkillTargets&) override;
        bool Ultimate(Fight::Fight_Logic&, int,
            const Attribute::AttackForm&, const Fight::SkillTargets&) override;
    };

}
