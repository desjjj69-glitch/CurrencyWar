#include"DataDrivenSkill.h"
#include "../BasicActor.h"
#include "../BattleContext.h"
#include "../GetRandomNum.h"
#include <algorithm>
#include <vector>

namespace Fight {

    void DataDrivenSkill::Execute(BasicActor* user,
        const SkillTargets& targets,
        BattleContext& ctx) {
        if (form.energyGain > 0)
            ctx.GainEnergy(user, form.energyGain);

        //per-hit multiplier: multiply[] is authoritative, fall back to atkScale
        auto MultiplierAt = [&](int hit) -> double {
            if (form.multiply.empty())
                return form.atkScale > 0.0 ? form.atkScale : 1.0;
            const size_t i = static_cast<size_t>(hit);
            return i < form.multiply.size() ? form.multiply[i] : form.multiply.back();
            };

        if (form.HasDamage() && !targets.list.empty()) {
            if (form.target == Attribute::TargetAttackType::Bounce) {
                //bounce: hitCount hits total, each hit picks a random living target
                std::vector<TargetInfo> alive;
                alive.reserve(targets.list.size());
                for (const auto& t : targets.list) {
                    if (t.actor && t.actor->Alive()) alive.push_back(t);
                }

                for (int hit = 0; hit < form.hitCount && !alive.empty(); ++hit) {
                    const size_t idx = static_cast<size_t>(
                        GetRandomNum(0, static_cast<int>(alive.size()) - 1));
                    ctx.DealDamage(user, alive[idx].actor, form, MultiplierAt(hit));

                    //drop targets killed by this hit so later hits are not wasted
                    alive.erase(
                        std::remove_if(alive.begin(), alive.end(),
                            [](const TargetInfo& t) {
                                return !t.actor || !t.actor->Alive();
                            }),
                        alive.end());
                }
            }
            else {
                //single / aoe / blast: every target takes all hitCount hits
                for (const auto& t : targets.list) {
                    if (!t.actor) continue;
                    for (int hit = 0; hit < form.hitCount; ++hit) {
                        if (!t.actor->Alive()) break;
                        ctx.DealDamage(user, t.actor, form, MultiplierAt(hit));
                    }
                }
            }
        }

        //non-damage effect: speed boost, BattleContext also repositions the action bar
        if (form.speedBoostPercent > 0.0) {
            for (const auto& t : targets.list) {
                if (!t.actor || !t.actor->Alive()) continue;
                ctx.ApplySpeedBoost(t.actor, form.speedBoostPercent);
            }
        }
    }
}
