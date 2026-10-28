#include "Herta.h"
#include "ActorSkillRegistry.h"
#include "ActorSkillHelpers.h"
#include "../BattleEventRegistry.h"
#include "../Fight_Logic.h"
#include "../Fight_CharacterActor.h"
#include "../Fight_EnemyActor.h"
#include "../SkillTargets.h"

REGISTER_ACTOR_SKILLS("Herta", Skill::Impl::HertaSkillSet);
REGISTER_BATTLE_EVENT_HANDLER("HertaTalent", Skill::Impl::HertaTalent);

namespace Skill::Impl {

    namespace {
        Fight::Character_Actor* FindCaster(Fight::Fight_Logic& logic, int casterId) {
            return dynamic_cast<Fight::Character_Actor*>(logic.FindActorById(casterId));
        }

        bool CastDamage(Fight::Fight_Logic& logic,
            int casterId,
            const Attribute::AttackForm& form,
            const Fight::SkillTargets& targets) {
            auto* self = FindCaster(logic, casterId);
            if (!self) return false;
            if (form.energyGain > 0.0) logic.GainEnergy(self, form.energyGain);
            Helpers::DamageTargets(logic, *self, form, targets);
            return true;
        }
    }

    bool HertaSkillSet::Normal(Fight::Fight_Logic& logic, int casterId,
        const Attribute::AttackForm& form, const Fight::SkillTargets& targets) {
        return CastDamage(logic, casterId, form, targets);
    }

    bool HertaSkillSet::Skill(Fight::Fight_Logic& logic, int casterId,
        const Attribute::AttackForm& form, const Fight::SkillTargets& targets) {
        return CastDamage(logic, casterId, form, targets);
    }

    bool HertaSkillSet::Ultimate(Fight::Fight_Logic& logic, int casterId,
        const Attribute::AttackForm& form, const Fight::SkillTargets& targets) {
        return CastDamage(logic, casterId, form, targets);
    }

    bool HertaSkillSet::Talent(Fight::Fight_Logic& logic, int casterId,
        const Attribute::AttackForm& form, const Fight::SkillTargets& targets) {
        auto* self = FindCaster(logic, casterId);
        if (!self) return false;
        Helpers::DamageBounce(logic, *self, form, targets, form.energyGainPerHit);
        return true;
    }

    HertaTalent::HertaTalent(int ownerId, Fight::BasicSkill* skill)
        : ownerId_(ownerId), skill_(skill) {
    }

    void HertaTalent::OnEvent(BattleEvent::IContext& ctx, const BattleEvent::Event& e) {
        if (!skill_) return;
        if (e.type != BattleEvent::Type::AfterDamage) return;
        if (e.skillId == skill_->form.id) return; // its own damage cannot retrigger it

        auto* logic = dynamic_cast<Fight::Fight_Logic*>(&ctx);
        if (!logic) return;

        auto* owner = FindCaster(*logic, ownerId_);
        if (!owner || !owner->Alive()) return;

        const double threshold = skill_->form.triggerHpThreshold;
        if (threshold <= 0.0) return;

        auto& enemies = logic->Actor(Fight::Fight_Logic::PlaceType::Enemy);

        // Re-arm enemies that are back at/above the threshold, and locate the
        // enemy affected by this event.
        Fight::Enemy_Actor* target = nullptr;
        for (auto& var : enemies) {
            auto* enemy = dynamic_cast<Fight::Enemy_Actor*>(var.get());
            if (!enemy) continue;
            if (enemy->CurrentHeart <= 0 || enemy->MaxHeart <= 0) {
                triggered_.erase(enemy->id());
                continue;
            }
            const double ratio = static_cast<double>(enemy->CurrentHeart) / enemy->MaxHeart;
            if (ratio >= threshold) triggered_.erase(enemy->id());
            if (enemy->id() == e.targetId) target = enemy;
        }
        if (!target) return;

        const double targetRatio =
            static_cast<double>(target->CurrentHeart) / target->MaxHeart;
        if (targetRatio >= threshold) return;
        if (triggered_.find(target->id()) != triggered_.end()) return;

        // Mark every enemy that is already below the threshold before the
        // talent fires, so its own damage cannot trigger itself again.
        for (auto& var : enemies) {
            auto* enemy = dynamic_cast<Fight::Enemy_Actor*>(var.get());
            if (!enemy || enemy->CurrentHeart <= 0 || enemy->MaxHeart <= 0) continue;
            const double ratio = static_cast<double>(enemy->CurrentHeart) / enemy->MaxHeart;
            if (ratio < threshold) triggered_.insert(enemy->id());
        }

        Fight::SkillTargets targets;
        size_t index = 0;
        targets.list.push_back({ target, index++, Fight::TargetRole::Main, 1.0 });
        for (auto& var : enemies) {
            auto* enemy = dynamic_cast<Fight::Enemy_Actor*>(var.get());
            if (!enemy || enemy->CurrentHeart <= 0) continue;
            if (enemy->id() == target->id()) continue;
            targets.list.push_back({ enemy, index++, Fight::TargetRole::Sub, 1.0 });
        }

        // The bus queues the talent's own AfterDamage events; they are ignored
        // by the skillId check at the top of this function.
        skill_->Execute(owner, targets, *logic);

        // Enemies brought below the threshold by this talent are marked only.
        for (auto& var : enemies) {
            auto* enemy = dynamic_cast<Fight::Enemy_Actor*>(var.get());
            if (!enemy || enemy->CurrentHeart <= 0 || enemy->MaxHeart <= 0) continue;
            const double ratio = static_cast<double>(enemy->CurrentHeart) / enemy->MaxHeart;
            if (ratio < threshold) triggered_.insert(enemy->id());
        }
    }

}
