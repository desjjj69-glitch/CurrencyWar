#pragma once
#include "IActorSkillSet.h"

namespace Skill::Impl {

    class AstaSkillSet : public IActorSkillSet {
    protected:
        bool Normal(Fight::Fight_Logic&, int,
            const Attribute::AttackForm&, const Fight::SkillTargets&) override;
        bool Skill(Fight::Fight_Logic&, int,
            const Attribute::AttackForm&, const Fight::SkillTargets&) override;
        bool Ultimate(Fight::Fight_Logic&, int,
            const Attribute::AttackForm&, const Fight::SkillTargets&) override;
    };

}
