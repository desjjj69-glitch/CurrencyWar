#pragma once
#include<string>
#include<algorithm>
#include"EquipmentType.h"
#include"../BasicObject.h"
// 装备在备战阶段同样存放在 Logic::zone_ 里，因此继承 BasicObject
class BasicEquipment :public BasicObject {
	inline static int GloabalIndex_Equipment = 0;
public:
	virtual ~BasicEquipment() = 0;
	BasicEquipment() {
		id = GloabalIndex_Equipment++;
	}
	WaitingForFight::ResourceType Type() const override {
		return WaitingForFight::ResourceType::Equipment;
	}
	// 装备类型（Normal/Advanced/Privilege/Consumable）。不能叫 Type()，会和 BasicObject::Type() 的返回类型冲突
	virtual EquipmentType Kind() const = 0;

	std::string GetName() const { return name; };
	void SetName(const std::string& n) { name = n; }
	void ResetStack() { stacks = 0; }
	void StackPlus() { 
		stacks = std::min(++stacks, stacklimit);
	}
	void SetStackLimit(int limit) { stacklimit = limit; }
	int GetId() const{ return id; }
private:
	std::string name;
	int id=-1;
	int stacks = 0;
	int stacklimit = 99;
};