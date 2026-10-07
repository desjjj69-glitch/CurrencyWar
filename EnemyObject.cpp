#include"EnemyObject.h"
#include"Logger.h"
#include"json.hpp"
#include"GetRandomNum.h"
#include"ParseSkill.h"
#include<fstream>
#include<filesystem>
using Logger::Log;
using json = nlohmann::json;
WaitingForFight::ResourceType EnemyObject::Type()const {
	return WaitingForFight::ResourceType::Enemy;
}
namespace Object {
	std::unordered_map<std::string, EnemyObject>Enemy_DefaultSetting;
    

    static Attribute::AttackForm ParseAttackForm(const json& j) {
        Attribute::AttackForm form;

        form.id = j.value("id", std::string(""));
        form.name = j.value("name", std::string(""));

        form.slot = ParseSkillSlot(j.value("slot", std::string("Normal")));
        form.target = ParseTargetAttackType(j.value("target", std::string("Single")));
        form.element = ParseElement(j.value("element", std::string("Physic")));

        if (j.contains("types") && j["types"].is_array()) {
            for (const auto& t : j["types"]) {
                if (t.is_string()) {
                    form.types.push_back(ParseAttackType(t.get<std::string>()));
                }
            }
        }
        if (form.types.empty()) {
            form.types.push_back(Attribute::AttackType::Normal);
        }

        form.atkScale = j.value("atkScale", 0.0);
        form.hpScale = j.value("hpScale", 0.0);
        form.defScale = j.value("defScale", 0.0);
        form.hitCount = j.value("hitCount", 1);
        form.breakValue = j.value("breakValue", 0.0);
        form.energyGain = j.value("energyGain", 0.0);

        return form;
    }
	static void LoadAttribute(Attribute::Enemy_Attribute& attr, const json& j) {
		if (!j.is_object()) return;
		if (j.contains("Heart"))   attr.heart = j.at("Heart").get<Attribute::HeartType>();
		if (j.contains("Attack"))  attr.attack = j.at("Attack").get<Attribute::AttackValueType>();
		if (j.contains("Defense")) attr.defense = j.at("Defense").get<Attribute::DefenseType>();
		if (j.contains("Speed"))   attr.speed = j.at("Speed").get<Attribute::SpeedType>();
		if (j.contains("Star"))    attr.star = j.at("Star").get<int>();
		if (j.contains("Level"))   attr.Level = j.at("Level").get<int>();
        if (j.contains("Skills") && j["Skills"].is_array()) {
            attr.Skills.clear();   
            for (const auto& skillJson : j["Skills"]) {
                if (!skillJson.is_object()) continue;
                attr.Skills.push_back(ParseAttackForm(skillJson));
            }
        }
    }
    void Enemy_LoadinJson(const std::filesystem::path& gp, std::vector<EnemyObject>& Boss) {

        Boss.clear();
        Boss.resize(3);
        std::ifstream configin(gp / "data" / "Enemy.json");
        json js;
        try {
            configin >> js;
        }
        catch (const json::parse_error& e) {
            Log(e.what(), __func__, Logcode::PARSEERROR);
        }
        catch (const std::exception& e) {
            Log(e.what(), __func__, Logcode::ERROR);
        }

        // ==================== 1. 读入 3 个 boss ====================
        if (js.contains("bosses") && js["bosses"].is_array()) {
            auto& jsboss = js["bosses"];
            size_t size = jsboss.size();
            auto getvec = GetRandomDistinct(static_cast<int>(size), 3);

            for (size_t i = 0; i < getvec.size(); i++) {
                auto& bossattr = jsboss[getvec[i]];
                if (!bossattr.is_object()) continue;

                Boss[i].EnemyType = bossattr.at("type").get<std::string>();
                Boss[i].name = bossattr.at("name").get<std::string>();

                //  boss 自身的属性
                if (bossattr.contains("Attribute")) {
                    LoadAttribute(Boss[i].attribute, bossattr["Attribute"]);
                }

                // 同步到默认表
                auto& def = Enemy_DefaultSetting[Boss[i].name];
                def.EnemyType = Boss[i].EnemyType;
                def.name = Boss[i].name;
                def.attribute = Boss[i].attribute;

                // 记录 mobIds
                if (bossattr.contains("mobIds") && bossattr["mobIds"].is_array()) {
                    for (auto& var : bossattr["mobIds"]) {
                        if (var.is_string())
                            Boss[i].mobs.push_back(var.get<std::string>());
                    }
                }
            }
        }

        // ============ 2. 只读取 boss 里出现过的小怪属性 ============
        if (js.contains("mobs") && js["mobs"].is_object()) {
            auto& jsmob = js["mobs"];

            for (size_t i = 0; i < Boss.size(); i++) {
                for (size_t j = 0; j < Boss[i].mobs.size(); j++) {
                    const std::string& mobId = Boss[i].mobs[j];
                    if (!jsmob.contains(mobId)) continue;   // JSON 里没有就跳过

                    const auto& mobJson = jsmob[mobId];
                    if (!mobJson.is_object()) continue;

                    auto& target = Enemy_DefaultSetting[mobId]; // 不存在则默认构造
                    target.EnemyType = mobJson.at("type").get<std::string>();
                    target.name = mobJson.at("name").get<std::string>();

                    //  小怪自身的属性
                    if (mobJson.contains("Attribute")) {
                        LoadAttribute(target.attribute, mobJson["Attribute"]);
                    }
                }
            }
        }
    }
    
    
}
