#pragma once
#include "BattleEventBus.h"
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

namespace Fight { class BasicSkill; }

namespace BattleEvent {

    // A handler is constructed per (skill, owner) at fight start.
    using HandlerFactory =
        std::function<std::unique_ptr<IHandler>(int ownerId, Fight::BasicSkill* skill)>;

    class Registry {
    public:
        static Registry& Instance();

        void Register(const std::string& id, HandlerFactory factory);
        std::unique_ptr<IHandler> Make(const std::string& id,
            int ownerId,
            Fight::BasicSkill* skill) const;

    private:
        std::unordered_map<std::string, HandlerFactory> factories_;
    };

    struct AutoRegister {
        AutoRegister(const std::string& id, HandlerFactory factory) {
            Registry::Instance().Register(id, std::move(factory));
        }
    };

}

#define REGISTER_BATTLE_EVENT_HANDLER(Id, Class) \
    static ::BattleEvent::AutoRegister _auto_reg_battle_event_##__COUNTER__( \
        Id, [](int ownerId, ::Fight::BasicSkill* skill) { \
            return std::unique_ptr<::BattleEvent::IHandler>( \
                std::make_unique<Class>(ownerId, skill)); \
        })
