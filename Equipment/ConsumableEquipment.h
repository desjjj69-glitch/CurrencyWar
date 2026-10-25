#pragma once
#include"BasicEquipment.h"
class ConsumableEquipment :public BasicEquipment {
public:
	~ConsumableEquipment() override = default;
	EquipmentType Kind() const override;
};
