#pragma once
#include "StrategyBus.h"
#include"../json.hpp"
namespace Strategy::Impl {
    class Strategy_13 : public IStrategy {
    public:
        explicit Strategy_13(const nlohmann::json& p);
        std::string Id() const override { return "Strategy_13"; }
        void OnEvent(IGameContext& ctx, const Event& e) override;
    };
}
