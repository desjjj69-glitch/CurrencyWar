#pragma once
#include "IActorSkillSet.h"
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

namespace Skill {

    using ActorSkillFactory = std::function<std::unique_ptr<IActorSkillSet>()>;

    class Registry {
    public:
        static Registry& Instance();

        void Register(const std::string& actorName, ActorSkillFactory factory);
        std::unique_ptr<IActorSkillSet> Make(const std::string& actorName) const;

    private:
        std::unordered_map<std::string, ActorSkillFactory> factories_;
    };

    struct AutoRegister {
        AutoRegister(const std::string& actorName, ActorSkillFactory factory) {
            Registry::Instance().Register(actorName, std::move(factory));
        }
    };

}

#define REGISTER_ACTOR_SKILLS(ActorName, Class) \
    static ::Skill::AutoRegister _auto_reg_actor_skills_##__COUNTER__( \
        ActorName, []() { return std::unique_ptr<::Skill::IActorSkillSet>( \
            std::make_unique<Class>()); })
