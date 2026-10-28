#include "ActorSkillRegistry.h"

namespace Skill {

    Registry& Registry::Instance() {
        static Registry registry;
        return registry;
    }

    void Registry::Register(const std::string& actorName, ActorSkillFactory factory) {
        factories_[actorName] = std::move(factory);
    }

    std::unique_ptr<IActorSkillSet> Registry::Make(const std::string& actorName) const {
        auto it = factories_.find(actorName);
        if (it == factories_.end()) return nullptr;
        return it->second();
    }

}
