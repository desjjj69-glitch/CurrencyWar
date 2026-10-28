#pragma once
#include "../BattleEventBus.h"
#include "../BasicSkill.h"
#include <string>
#include <unordered_set>

namespace Skill::Impl {

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
