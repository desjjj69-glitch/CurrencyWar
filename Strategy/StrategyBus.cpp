// Strategy/StrategyBus.cpp
#include "StrategyBus.h"
#include"StrategyRegistry.h"
namespace Strategy {

    Bus& Bus::Instance() {
        static Bus b;
        return b;
    }
  
    void Bus::AddStageStrategy(std::unique_ptr<IStrategy> s) { 
        if (!s)return;
        stage_.push_back(std::move(s));
    }
    void Bus::AddPermanentStrategy(std::unique_ptr<IStrategy> s) { 
        if (!s)return;
        permanent_.push_back(std::move(s)); 
    }
    void Bus::AddStageStrategy(const std::string& id) {
        AddStageStrategy(Registry::Instance().Make(id));
    }
    void Bus::AddPermanentStrategy(const std::string& id) { 
        AddPermanentStrategy(Registry::Instance().Make(id));
    }
    void Bus::ClearPermanentStrategies() {
        permanent_.clear();
    }
    std::vector<PermanentState> Bus::SavePermanentStates() const {
        std::vector<PermanentState> states;
        states.reserve(permanent_.size());
        for (const auto& strategy : permanent_) {
            if (!strategy) continue;
            PermanentState state;
            state.id = strategy->Id();
            state.state = strategy->SaveState();
            states.push_back(std::move(state));
        }
        return states;
    }
    void Bus::LoadPermanentStates(const std::vector<PermanentState>& states) {
        permanent_.clear();
        for (const auto& state : states) {
            auto strategy = Registry::Instance().Make(state.id);
            if (!strategy) continue;
            strategy->LoadState(state.state);
            permanent_.push_back(std::move(strategy));
        }
    }

    void Bus::BeginStage() { /* 根据玩家本阶段的选择构造 stage_ */ }
    void Bus::EndStage() { /*stage_.clear();*/ } //现在的策略都是永久的，跨阶段的，不要删掉

    void Bus::Emit(IGameContext& ctx, const Event& e) {
        pending_.push_back(e);
        if (depth_ > 0) return;   //策略正在执行时不允许emit        
        ++depth_;
        while (!pending_.empty()) {
            auto ev = std::move(pending_.back());
            pending_.pop_back();
            for (auto& s : stage_)     if (!s->IsConsumed()) s->OnEvent(ctx, ev);
            for (auto& s : permanent_) s->OnEvent(ctx, ev);
        }
        --depth_;
        std::erase_if(stage_, [](auto& s) { return s->IsConsumed(); });
    }

} // namespace Strategy