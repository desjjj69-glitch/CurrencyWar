#pragma once
#include <string>

namespace BattleEvent {

    // Battle-internal events. New event kinds can be added here and emitted
    // from the corresponding point in Fight_Logic without touching handlers.
    enum class Type {
        FightBegin,
        TurnStart,
        TurnEnd,
        BeforeDamage,
        AfterDamage,
        EnemyEnter,
        EnemyDeath,
        SkillUsed,
        FightEnd
    };

    struct Event {
        Type type = Type::FightBegin;
        int sourceId = -1;          // actor that caused the event
        int targetId = -1;          // actor affected by the event
        std::string skillId;        // skill that caused the event, if any
        double amount = 0.0;        // damage / generic numeric payload
        double targetHpRatio = -1.0; // target HP / MaxHP after the event
        int aux = 0;                // extra payload (win/lose, indexes, ...)
    };

}
