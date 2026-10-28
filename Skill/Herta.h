#pragma once
#include "IActorSkillSet.h"
#include "../BattleEventBus.h"
#include "../BasicSkill.h"
#include <string>
#include <unordered_set>

namespace Skill::Impl {

    class HertaSkillSet : public IActorSkillSet {
    protected:
        bool Normal(Fight::Fight_Logic&, int,
            const Attribute::AttackForm&, const Fight::SkillTargets&) override;
        bool Skill(Fight::Fight_Logic&, int,
            const Attribute::AttackForm&, const Fight::SkillTargets&) override;
        bool Ultimate(Fight::Fight_Logic&, int,
            const Attribute::AttackForm&, const Fight::SkillTargets&) override;
        bool Talent(Fight::Fight_Logic&, int,
            const Attribute::AttackForm&, const Fight::SkillTargets&) override;
    };

    // Herta's talent: when an enemy drops below the configured HP threshold,
    // immediately perform one bounce attack.
    class HertaTalent : public BattleEvent::IHandler {
    public:
        HertaTalent(int ownerId, Fight::BasicSkill* skill);

        std::string Id() const override { return "HertaTalent"; }
        int OwnerId() const override { return ownerId_; }
        void OnEvent(BattleEvent::IContext& ctx, const BattleEvent::Event& e) override;

    private:
        int ownerId_ = -1;
        Fight::BasicSkill* skill_ = nullptr;
        std::unordered_set<int> triggered_;
    };

}
