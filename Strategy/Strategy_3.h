// 环境策略 Strategy_3：位面切换时按位面发放 Money 晶矿
#pragma once
#include "StrategyBus.h"
#include"../json.hpp"
namespace Strategy::Impl {
    class Strategy_3 : public IStrategy {
    public:
        explicit Strategy_3(const nlohmann::json& p);
        std::string Id() const override { return "Strategy_3"; }
        void OnEvent(IGameContext& ctx, const Event& e) override;
    };
}
