#include "BattleEventRegistry.h"
#include "Logger.h"

namespace BattleEvent {

    Registry& Registry::Instance() {
        static Registry registry;
        return registry;
    }

    void Registry::Register(const std::string& id, HandlerFactory factory) {
        factories_[id] = std::move(factory);
    }

    std::unique_ptr<IHandler> Registry::Make(const std::string& id,
        int ownerId,
        Fight::BasicSkill* skill) const {
        auto it = factories_.find(id);
        if (it == factories_.end()) {
            Logger::Log("battle event handler not registered: " + id,
                __func__, Logcode::NOTFOUND);
            return nullptr;
        }
        return it->second(ownerId, skill);
    }

}
