#pragma once
#include<string>
#include"EquipmentType.h"
class BasicEquipment {
	inline static int GloabalIndex_Equipment = 0;
public:
	virtual ~BasicEquipment() = 0;
	BasicEquipment() {
		id = GloabalIndex_Equipment++;
	}
	virtual EquipmentType Type() = 0;

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