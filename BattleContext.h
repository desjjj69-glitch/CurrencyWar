
#pragma once
#include <string>
#include"AttackForm.h"
namespace Fight {
    class BasicActor;
    class BattleContext {
    public:
        virtual ~BattleContext() = default;
        virtual void DealDamage(BasicActor* caster, BasicActor* target,
            const Attribute::AttackForm& form,
            double multiplier) = 0;
        virtual void GainEnergy(BasicActor* who, double amount) = 0;
        virtual void ApplyBuff(BasicActor* target, const std::string& buffId) = 0;
        //按百分比提升目标速度（0.15 = +15%），并同步修正其在行动条上的位置
        virtual void ApplySpeedBoost(BasicActor* target, double percent) = 0;
        
    };
}
