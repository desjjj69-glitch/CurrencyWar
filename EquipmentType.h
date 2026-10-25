#pragma once
#include<string>
#include<sstream>
enum class EquipmentType {
	Normal,
	Advanced,
	Privilege,
	Consumable
};
inline EquipmentType ParseEquipmentType(const std::string& s) {
	using E=EquipmentType;
	if (s == "Normal" || s == "Basic")return E::Normal;
	else if (s == "Advanced")return E::Advanced;
	else if (s == "Privilege")return E::Privilege;
	else if (s == "Consumable")return E::Consumable;
	return E::Normal;
}