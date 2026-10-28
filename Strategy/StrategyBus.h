// Strategy/StrategyBus.h
#pragma once
#include <string>
#include <vector>
#include <memory>
#include <algorithm>

namespace Strategy {

    // 所有阶段的领域事件统一放这里，用前缀区分来源
    enum class EventType {
        // ---- 备战阶段 ----
        PrepStageBegin,
        PrepStageEnd,
        DimensionChanged,
        WinStreakChanged,
        ShopRefreshed,
        CharacterBought,
        CharacterSold,
        LevelUpgraded,
        StarUpgraded,
        ExpBought,
        MineralOpened,
        FightStarted,

        // ---- 战斗阶段 ----
        FightBegin,
        RoundBegin,
        RoundEnded,
        SkillCast,
        CharacterDied,
        FightEnded,

        // ---- 结算阶段 ----
        SettleBegin,
        BattleWon,
        BattleLost,
        SettleEnded,
    };

    struct Event {
        EventType type;
        std::string actorName;   // 角色名 / 装备名，无则空
        int         amount = 0; // 金币、伤害、回合数等
        int         aux = 0; // 备用字段，视事件语义
    };

    // 策略能操作的游戏状态 —— 只暴露"各阶段都能安全操作"的方法
    enum class StageKind{WaitingForFight,Fight,FightResult};
    class IGameContext {
    public:
        virtual ~IGameContext() = default;

        virtual StageKind Kind() const= 0;
    };

    class IStrategy {
    public:
        virtual ~IStrategy() = default;
        virtual std::string Id() const = 0;
        virtual bool IsConsumed() const { return false; }  // 一次性海克斯返回 true
        virtual void OnEvent(IGameContext& ctx, const Event& e) = 0;
    };

    class Bus {
    public:
        static Bus& Instance();

        void AddStageStrategy(std::unique_ptr<IStrategy> s);
        void AddPermanentStrategy(std::unique_ptr<IStrategy> s);
        void AddStageStrategy(const std::string&);
        void AddPermanentStrategy(const std::string&);
        void ClearPermanentStrategies();
        // 由阶段切换器调用：进入/离开某个阶段时，管理 stage 级策略的生命周期
        void BeginStage();
        void EndStage();

        // 由各阶段 Logic 在自己的变更点调用；ctx 就是那个 Logic 自己
        void Emit(IGameContext& ctx, const Event& e);

    private:
        std::vector<std::unique_ptr<IStrategy>> stage_, permanent_;
        std::vector<Event> pending_;
        int depth_ = 0;
    };

} // namespace Strategy