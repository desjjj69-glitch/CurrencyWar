// StrategyEvent.h
#pragma once
#include <string>
#include <vector>
#include <memory>

namespace WaitingForFight {

    class Logic;

    enum class StrategyEventType {
        StageBegin,        // Init() 进入新节点/阶段
        StageEnd,          // DataHandOff 之前
        ShopRefreshed,     // shop.Dcard 之后
        CharacterBought,   // shop.Buy 成功
        CharacterSold,     // shop.Sell
        LevelUpgraded,     // shop.LevelUp 成功
        StarUpgraded,      // StarUp() 真的升了
        ExpBought,         // shop.BuyExp 成功
        MineralOpened,     // HandleMineralPress 结算完
        FightStarted,      // 点击 StartFight 成功
    };

    struct StrategyEvent {
        StrategyEventType type;
        std::string characterName;   // 买/卖角色时填
        int characterCost = 0;
        int amount = 0;              // 金币/经验变化量
    };

    class IStrategy {
    public:
        virtual ~IStrategy() = default;
        virtual std::string Id() const = 0;
        virtual void OnEvent(Logic& logic, const StrategyEvent& e) = 0;
    };

}