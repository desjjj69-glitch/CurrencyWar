#pragma once
#include "StrategyBus.h"
#include"../json.hpp"
namespace Strategy::Impl {
    class Strategy_9 : public IStrategy {
    public:
        explicit Strategy_9(const nlohmann::json& p);
        std::string Id() const override { return "Strategy_9"; }
        void OnEvent(IGameContext& ctx, const Event& e) override;
        nlohmann::json SaveState() const override;
        void LoadState(const nlohmann::json& state) override;
    private:
        bool granted_ = false;
    };
}
