#pragma once
#include "BasicObject.h"
#include"CharacterAttribute.h"
#include"Equipment/BasicEquipment.h"
#include<string>
#include<unordered_map>
#include<filesystem>
#include<memory>
#include<vector>

class CharacterObject :public BasicObject {
public:
	
	~CharacterObject() = default;
	WaitingForFight::ResourceType Type()const override;
	std::string name;
	std::string general_description;
	std::string detailed_description;
	Attribute::CharacterGameAttribute game_attribute;
	Attribute::CharacterBasicAttribute basic_attribute;
	void NewCharacter(const std::string&);

	//装备栏，上限 3
	static constexpr size_t EquipmentSlotLimit = 3;
	bool HasEquipmentSpace()const { return equipments.size() < EquipmentSlotLimit; }
	bool AddEquipment(std::unique_ptr<BasicEquipment>& equipment) {
		if (!equipment || !HasEquipmentSpace()) return false;
		equipments.push_back(std::move(equipment));
		return true;
	}
	std::vector<std::unique_ptr<BasicEquipment>>& GetEquipments() { return equipments; }
	const std::vector<std::unique_ptr<BasicEquipment>>& GetEquipments()const { return equipments; }
private:
	std::vector<std::unique_ptr<BasicEquipment>> equipments;
};
namespace Object {
	extern std::unordered_map<std::string, CharacterObject> Character_DefaultSetting;
	void Character_LoadinJson(const std::filesystem::path&);
}
