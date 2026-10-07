#pragma once
#include"BasicObject.h"
#include"AttributeTypes.h"
#include"AttackForm.h"
#include<string>
#include<unordered_map>
#include<vector>
#include<filesystem>
namespace Attribute {
	class Enemy_Attribute {
	public:
		HeartType heart=200000;
		AttackValueType attack=5000;
		DefenseType defense = 1000;
		SpeedType speed = 100;
		int star = 1;
		int Level = 90;
		std::unordered_map<ElementType, double>Resistance_ElementDamageResistance;
		std::vector<AttackForm>Skills;
	};
}
class EnemyObject :public BasicObject {
public:
	~EnemyObject() = default;
	WaitingForFight::ResourceType Type()const override;
	std::string EnemyType;//Boss/小怪/扑满
	std::string name;
	Attribute::Enemy_Attribute attribute;
	std::vector<std::string> mobs;//如果是boss就启用这个数组
	
};
namespace Object {
	extern std::unordered_map<std::string, EnemyObject>Enemy_DefaultSetting;
	void Enemy_LoadinJson(const std::filesystem::path&, std::vector<EnemyObject>&);//先读入3个boss再说
	
}