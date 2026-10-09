#pragma once
#include"BasicActor.h"
#include"EnemyObject.h"
#include<string>
namespace Fight {
	class Enemy_Actor :public BasicActor {//基类有速度和id
	public:
		ActorType Type()const override { return ActorType::Enemy; };
		std::string Name()const override { return name; }
		Enemy_Actor() = default;
		std::string name;
		EnemyObject config;
		//std::vector<Attribute::BuffInstance> RealTimeBuffs;
		//目前没有设置敌人的buff
		Attribute::HeartType CurrentHeart = 0;
		Attribute::HeartType MaxHeart = 0;
		void NewActor(const std::string&,int);
	};
}