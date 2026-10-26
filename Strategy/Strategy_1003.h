#pragma once
#include "StrategyBus.h"
#include"../json.hpp"
namespace Strategy::Impl {
    class Strategy_1003 : public IStrategy {
    public:
        explicit Strategy_1003(const nlohmann::json& p);
        std::string Id() const override { return "Strategy_1003"; }
        void OnEvent(IGameContext& ctx, const Event& e) override;
    };
}
