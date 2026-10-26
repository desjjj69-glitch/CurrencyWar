// 环境策略 Strategy_4：首次连胜达到 5 时发放装备
#pragma once
#include "StrategyBus.h"
#include"../json.hpp"
namespace Strategy::Impl {
    class Strategy_4 : public IStrategy {
    public:
        explicit Strategy_4(const nlohmann::json& p);
        std::string Id() const override { return "Strategy_4"; }
        void OnEvent(IGameContext& ctx, const Event& e) override;
    private:
        bool granted_ = false;
    };
}
