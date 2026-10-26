// 原 TestStrategy3，现环境策略 Strategy_6
#pragma once
#include "StrategyBus.h"
#include"../json.hpp"
namespace Strategy::Impl {
    class Strategy_6 : public IStrategy {
    public:
        explicit Strategy_6(const nlohmann::json& p);
        std::string Id() const override { return "Strategy_6"; }
        void OnEvent(IGameContext& ctx, const Event& e) override;
    private:
        int refresh_count_ = 0;
        bool granted_ = false;
    };
}
