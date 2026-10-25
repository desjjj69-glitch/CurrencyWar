#pragma once
#include"BasicEquipment.h"
class AdvancedEquipment :public BasicEquipment {
public:
	~AdvancedEquipment() override = default;
	EquipmentType Kind() const override;
};
