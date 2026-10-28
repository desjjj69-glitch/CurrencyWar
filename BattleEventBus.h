#pragma once
#include "BattleEvent.h"
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace Fight { class BasicSkill; }

namespace BattleEvent {

    // Context handed to handlers. Fight_Logic implements it and is the only
    // object that broadcasts; handlers dynamic_cast it back to Fight_Logic to
    // read/modify the battle.
    class IContext {
    public:
        virtual ~IContext() = default;
        virtual void Emit(const Event& e) = 0;
    };

    class IHandler {
    public:
        virtual ~IHandler() = default;
        virtual std::string Id() const = 0;
        virtual int OwnerId() const = 0;
        virtual void OnEvent(IContext& ctx, const Event& e) = 0;
    };

    class Bus {
    public:
        static Bus& Instance();

        // add/remove interfaces
        void AddHandler(std::unique_ptr<IHandler> handler);
        void RemoveHandlerByOwner(int ownerId);
        void Clear();
        std::size_t Size() const;

        // broadcast; nested emits are queued and drained after the current one
        void Emit(IContext& ctx, const Event& e);

    private:
        std::vector<std::unique_ptr<IHandler>> handlers_;
        std::vector<Event> pending_;
        std::vector<int> pendingRemoval_;
        int depth_ = 0;
    };

}
