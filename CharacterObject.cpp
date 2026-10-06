#include"CharacterObject.h"
#include<fstream>
#include"json.hpp"
#include"Logger.h"
using json = nlohmann::json;
using Logger::Log;
WaitingForFight::ResourceType CharacterObject::Type()const {
	return WaitingForFight::ResourceType::Character;
}
namespace Object{
	std::unordered_map<std::string, CharacterObject> Character_DefaultSetting;
	static Attribute::SuitPlace ParseSuitPlace(const std::string& s) {
		if (s == "Front")       return Attribute::SuitPlace::Front;
		if (s == "Back")        return Attribute::SuitPlace::Back;
		if (s == "Front_Back")  return Attribute::SuitPlace::Front_Back;
		return Attribute::SuitPlace::Front_Back; // 默认
	}

	static Attribute::ElementType ParseElement(const std::string& s) {
		if (s == "Physic")     return Attribute::ElementType::Physic;
		if (s == "Fire")       return Attribute::ElementType::Fire;
		if (s == "Ice")        return Attribute::ElementType::Ice;
		if (s == "Electro")    return Attribute::ElementType::Electro;
		if (s == "Wind")       return Attribute::ElementType::Wind;
		if (s == "Quantum")    return Attribute::ElementType::Quantum;
		if (s == "Imaginary")  return Attribute::ElementType::Imaginary;
		return Attribute::ElementType::Physic;
	}
    void Object::Character_LoadinJson(const std::filesystem::path& gp) {
        std::ifstream configin(gp/"data"/"BasicCharacterSettings.json");
        if (!configin.is_open()) return;

        json js;
        try {
            configin >> js;
        }
        catch (const std::exception& e) {
            Log(e.what(), __func__);
            return;
        }

        if (!js.contains("Characters") || !js["Characters"].is_array()) return;

        for (auto& var : js["Characters"]) {
            std::string key = var.value("Name", "Unknown");

            auto& ch = Character_DefaultSetting[key];
            ch.name = key;

            // ---- 顶层 ----
            ch.game_attribute.cost = var.value("Cost", 1);
            ch.game_attribute.stars = var.value("Star", 1);

            // ---- GameAttribute ----
            if (var.contains("GameAttribute") && var["GameAttribute"].is_object()) {
                const auto& ga = var["GameAttribute"];
                ch.game_attribute.suitplace =
                    ParseSuitPlace(ga.value("SuitPlace", std::string("Front_Back")));
            }

            // ---- BasicAttribute ----
            if (var.contains("BasicAttribute") && var["BasicAttribute"].is_object()) {
                const auto& ba = var["BasicAttribute"];
                auto& b = ch.basic_attribute;

                b.name = key;
                b.Level = ba.value("Level", 80);
                b.element = ParseElement(ba.value("Element", std::string("Physic")));
                b.BasicHeart = ba.value("Heart", 0);
                b.BasicAttack = ba.value("Attack", 0);
                b.BasicDefense = ba.value("Defense", 0);
                b.Speed = ba.value("Speed", 0.0);
                b.CritRate = ba.value("CritRate", 0.0);
                b.CritDamage = ba.value("CritDamage", 0.0);
                b.BreakEffect = ba.value("BreakEffect", 0.0);
                b.HealingBonus = ba.value("HealingBonus", 0.0);
                b.EnergyLimit = ba.value("EnergyLimit", 100);
                b.EnergyRecoverEfficiency = ba.value("EnergyRecoverEfficiency", 1.0);
                b.EffectHitRate = ba.value("EffectHitRate", 0.0);
                b.EffectResistance = ba.value("EffectResistance", 0.0);
                b.ElationRate = ba.value("ElationRate", 0.0);
            }
        }
    }
}
void CharacterObject::NewCharacter(const std::string& n) {
	name = n;
	game_attribute = Object::Character_DefaultSetting[n].game_attribute;
    basic_attribute = Object::Character_DefaultSetting[n].basic_attribute;
}