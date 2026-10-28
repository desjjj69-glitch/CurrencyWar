// SkillFactory.cpp
#include "SkillFactory.h"
#include "../BasicActor.h"

namespace Fight {

    namespace {
        // Thin wrapper stored in the actor's skill slots. Selection/targeting
        // still lives in Fight_Logic; the actual cast is dispatched to the
        // actor's IActorSkillSet implementation.
        class ActorSkill : public BasicSkill {
        public:
            explicit ActorSkill(const Attribute::AttackForm& f) { form = f; }

            void Execute(BasicActor* user,
                const SkillTargets& targets,
                BattleContext& ctx) override {
                if (!user) return;
                user->ExecuteSkill(form, targets, ctx);
            }
        };
    }

    std::unique_ptr<BasicSkill> CreateSkill(const Attribute::AttackForm& form) {
        return std::make_unique<ActorSkill>(form);
    }
}
