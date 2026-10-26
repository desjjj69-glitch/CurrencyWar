// Strategy/strategies/SurplusCapital.h
#pragma once
#include "StrategyBus.h"
#include"../json.hpp"
namespace Strategy::Impl {
    class TestStrategy1 : public IStrategy {
    public:
        explicit TestStrategy1(const nlohmann::json& /*p*/);
        std::string Id() const override { return "TestStrategy1"; }
        void OnEvent(IGameContext& ctx, const Event& e) override;
    };
}