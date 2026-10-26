// Strategy/AutoRegister.h
#pragma once
#include "StrategyRegistry.h"

namespace Strategy {
    struct AutoRegister {
        AutoRegister(const std::string& id, StrategyFactory f) {
            Registry::Instance().Register(id, std::move(f));
        }
    };
}

#define REGISTER_STRATEGY(Id, Class) \
    static ::Strategy::AutoRegister _auto_reg_##__COUNTER__( \
        Id, [](const nlohmann::json& p) { return std::make_unique<Class>(p); })