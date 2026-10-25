#pragma once
#include"BasicEquipment.h"
class NormalEquipment :public BasicEquipment {
public:
	~NormalEquipment() override = default;
	EquipmentType Kind() const override;//目前只有一个平凡实现，后续再补效果
};
