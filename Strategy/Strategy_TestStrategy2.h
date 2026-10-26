#pragma once
#include "StrategyBus.h"
#include"../json.hpp"
namespace Strategy::Impl {
    class TestStrategy2 : public IStrategy {
    public:
        explicit TestStrategy2(const nlohmann::json& p);
        std::string Id() const override { return "TestStrategy2"; }
        void OnEvent(IGameContext& ctx, const Event& e) override;
    };
}
