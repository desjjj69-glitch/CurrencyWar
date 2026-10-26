#pragma once
#include "StrategyBus.h"
#include"../json.hpp"
namespace Strategy::Impl {
    class TestStrategy3 : public IStrategy {
    public:
        explicit TestStrategy3(const nlohmann::json& p);
        std::string Id() const override { return "TestStrategy3"; }
        void OnEvent(IGameContext& ctx, const Event& e) override;
    };
}
