#pragma once
#include <vector>

namespace Fight {

    class BasicActor;

    enum class TargetRole {
        Main,       // 主目标，吃完整倍率
        Adjacent,   // 扩散相邻目标，吃衰减倍率
        Sub,        // 弹射/连锁的次级目标
        Self,       // 自己
        Ally,       // 友方
        All,        // 全体（一般不单独标记，直接展开成多个 Main/Ally）
        Count
    };

    struct TargetInfo {
        BasicActor* actor = nullptr;
        size_t index = 0;
        TargetRole role = TargetRole::Main;
        double damageMultiplier = 1.0;   // 相对 form 基础倍率的系数
    };

    struct SkillTargets {
        std::vector<TargetInfo> list;

        BasicActor* Main() const {
            for (const auto& t : list)
                if (t.role == TargetRole::Main) return t.actor;
            return list.empty() ? nullptr : list.front().actor;
        }

        bool Empty() const { return list.empty(); }
        size_t Size() const { return list.size(); }
    };
}