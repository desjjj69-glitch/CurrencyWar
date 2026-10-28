#pragma once
#include"BasicActor.h"
#include"CharacterObject.h"
#include"Skill/IActorSkillSet.h"
#include<vector>
#include<memory>
#include<string>
namespace Fight {
	
	class Character_Actor :public BasicActor {//基类有速度和id
	public:
		ActorType Type()const override { return ActorType::Character; };
		std::string Name()const override { return name; }
		bool Alive()const override { return heart > 0; }
		Character_Actor() = default;
		std::string name;
		CharacterObject config;
		std::vector<Attribute::BuffInstance> RealTimeBuffs;
		Attribute::HeartType heart=0;
		Attribute::HeartType MaxHeart = 0;
		int Energy=0;
		void NewActor(const CharacterObject&);//从备战阶段的角色构造新的战斗Actor
		bool ExecuteSkill(const Attribute::AttackForm&, const SkillTargets&, BattleContext&) override;
	private:
		std::unique_ptr<Skill::IActorSkillSet> skillImpl_;
	};
}