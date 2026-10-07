
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
        
    };
}
