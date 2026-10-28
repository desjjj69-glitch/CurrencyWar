#pragma once
#include"Fight_ActorType.h"
#include"BasicSkill.h"
#include<cmath>
#include<string>
#include<memory>
namespace Fight {
	static constexpr double ActionValueInaRound = 10000;
	class BasicActor {
	public:
		double ActionValue = -1;
		double Speed=0;
		virtual ActorType Type()const = 0;
		virtual std::string Name()const = 0;
		virtual bool Alive()const = 0;//是否还能被选为目标
		BasicActor() = default;
		int id()const { return id_; }
		void SetId(int id) { id_ = id; }
		BasicSkill* GetSkillById(const std::string&);
		BasicSkill* GetSkillBySlot(Attribute::SkillSlot);

		BasicSkill* GetSkillByIndex(size_t);
		const std::vector<std::unique_ptr<BasicSkill>>& Skills() const;
	private:
		
		int id_=-1;
	protected:
		std::vector<std::unique_ptr<BasicSkill>> skills;
	};
	enum class ExtraPriority {
		FieldAssist,
		CharacterCounter,
		EnemyCounter,
		CharacterUlt,
		CharacterEnforced,
		EnemyEnforced,
		Normal
	};
	struct ActorList {
		int id = -1;
		double ActionValue = -1;
		
	};
	struct ExtraActorEntry {
		int id = -1;
		ExtraPriority Priority = ExtraPriority::Normal;
	};
	inline bool operator<(const BasicActor& a, const BasicActor& b) {
		if (a.ActionValue != b.ActionValue) {
			return a.ActionValue < b.ActionValue;
		}
		return a.id() < b.id();
	}
	inline bool operator<(const ActorList& a, const ActorList& b) {
		if (a.ActionValue != b.ActionValue) {
			return a.ActionValue < b.ActionValue;
		}
		return a.id < b.id;
	}
}