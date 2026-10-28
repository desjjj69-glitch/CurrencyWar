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
    void Enemy_LoadinJson(const std::filesystem::path& gp,
        std::vector<EnemyObject>& Boss,
        std::vector<std::string>& name) {
        Boss.clear();
        Boss.resize(name.size());          // 以传入的名字数量为准，而不是硬编码 3

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

        // ---- 1. 建立 name -> bosses 元素 的索引 ----
        std::unordered_map<std::string, const json*> bossMap;
        if (js.contains("bosses") && js["bosses"].is_array()) {
            for (const auto& b : js["bosses"]) {
                if (!b.is_object()) continue;
                if (!b.contains("name") || !b["name"].is_string()) continue;
                bossMap[b["name"].get<std::string>()] = &b;
            }
        }

        // ---- 2. 按 GameStart 选好的顺序加载 ----
        for (size_t i = 0; i < name.size(); ++i) {
            auto it = bossMap.find(name[i]);
            if (it == bossMap.end()) {
                Log("Boss not found: " + name[i], __func__, Logcode::ERROR);
                continue;                  // 留空对象，不崩
            }
            const json& bossattr = *it->second;

            Boss[i].EnemyType = bossattr.at("type").get<std::string>();
            Boss[i].name = name[i];

            if (bossattr.contains("Attribute")) {
                LoadAttribute(Boss[i].attribute, bossattr["Attribute"]);
            }

            // 同步到默认表
            auto& def = Enemy_DefaultSetting[Boss[i].name];
            def.EnemyType = Boss[i].EnemyType;
            def.name = Boss[i].name;
            def.attribute = Boss[i].attribute;

            // 记录 mobIds
            Boss[i].mobs.clear();          // 防止重复调用时累积
            if (bossattr.contains("mobIds") && bossattr["mobIds"].is_array()) {
                for (auto& var : bossattr["mobIds"]) {
                    if (var.is_string())
                        Boss[i].mobs.push_back(var.get<std::string>());
                }
            }
        }

        // ---- 3. 小怪部分保持原样 ----
        if (js.contains("mobs") && js["mobs"].is_object()) {
            auto& jsmob = js["mobs"];
            for (size_t i = 0; i < Boss.size(); i++) {
                for (size_t j = 0; j < Boss[i].mobs.size(); j++) {
                    const std::string& mobId = Boss[i].mobs[j];
                    if (!jsmob.contains(mobId)) continue;
                    const auto& mobJson = jsmob[mobId];
                    if (!mobJson.is_object()) continue;

                    auto& target = Enemy_DefaultSetting[mobId];
                    target.EnemyType = mobJson.at("type").get<std::string>();
                    target.name = mobJson.at("name").get<std::string>();
                    if (mobJson.contains("Attribute")) {
                        LoadAttribute(target.attribute, mobJson["Attribute"]);
                    }
                }
            }
        }
    }
    
    
}
