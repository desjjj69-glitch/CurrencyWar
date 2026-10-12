#pragma once
#include<string>
#include<sstream>
enum class EquipmentType {
	Normal,
	Advanced,
	Privilege
};
inline EquipmentType ParseEquipmentType(const std::string& s) {
	using E=EquipmentType;
	if (s == "Normal")return E::Normal;
	else if (s == "Advanced")return E::Advanced;
	else if (s == "Privilege")return E::Privilege;
	return E::Normal;
}