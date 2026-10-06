#pragma once
#include "BasicObject.h"
#include"CharacterAttribute.h"
#include<string>
#include<unordered_map>
#include<filesystem>

class CharacterObject :public BasicObject {
public:
	
	~CharacterObject() = default;
	WaitingForFight::ResourceType Type()const override;
	std::string name;
	Attribute::CharacterGameAttribute game_attribute;
	Attribute::CharacterBasicAttribute basic_attribute;
	void NewCharacter(const std::string&);
};
namespace Object {
	extern std::unordered_map<std::string, CharacterObject> Character_DefaultSetting;
	void Character_LoadinJson(const std::filesystem::path&);
}
