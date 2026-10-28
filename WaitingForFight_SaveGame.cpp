#include "WaitingForFight_SaveGame.h"
#include "WaitingForFight_InteractLogic.h"
#include "Mineral.h"
#include "Equipment/BasicEquipment.h"
#include "CharacterObject.h"
#include "EnemyObject.h"
#include "StageData.h"
#include "GameStart_Logic.h"
#include "Strategy/StrategyRegistry.h"
#include "Strategy/StrategyBus.h"
#include "Logger.h"
#include "json.hpp"
#include <fstream>
#include <unordered_map>
#include <algorithm>

using json = nlohmann::json;
using Logger::Log;

namespace WaitingForFight::SaveGame {

    namespace {

        json SaveEquipment(const BasicEquipment& equipment) {
            json j;
            j["name"] = equipment.GetName();
            j["kind"] = static_cast<int>(equipment.Kind());
            j["stacks"] = equipment.GetStack();
            j["stackLimit"] = equipment.GetStackLimit();
            return j;
        }

        std::unique_ptr<BasicEquipment> LoadEquipment(const json& j) {
            if (!j.is_object()) return nullptr;
            const std::string name = j.value("name", std::string(""));
            if (name.empty()) return nullptr;

            EquipmentType type = EquipmentType::Normal;
            if (j.contains("kind") && j["kind"].is_number_integer()) {
                type = static_cast<EquipmentType>(j["kind"].get<int>());
            }
            else if (!Mineral::FindEquipmentQuality(name, type)) {
                type = EquipmentType::Normal;
            }

            auto equipment = Mineral::MakeEquipment(name, type);
            if (!equipment) {
                EquipmentType fallback = EquipmentType::Normal;
                if (Mineral::FindEquipmentQuality(name, fallback)) {
                    equipment = Mineral::MakeEquipment(name, fallback);
                }
            }
            if (!equipment) {
                Log("SaveGame: cannot rebuild equipment " + name, __func__, Logcode::NOTFOUND);
                return nullptr;
            }
            equipment->SetStackLimit(j.value("stackLimit", 99));
            equipment->SetStack(j.value("stacks", 0));
            return equipment;
        }

        json SaveCharacter(const CharacterObject& character) {
            json j;
            j["name"] = character.name;
            j["stars"] = character.game_attribute.stars;
            j["equipment"] = json::array();
            for (const auto& equipment : character.GetEquipments()) {
                if (equipment) j["equipment"].push_back(SaveEquipment(*equipment));
            }
            return j;
        }

        std::unique_ptr<CharacterObject> LoadCharacter(const json& j) {
            if (!j.is_object()) return nullptr;
            const std::string name = j.value("name", std::string(""));
            auto it = Object::Character_DefaultSetting.find(name);
            if (it == Object::Character_DefaultSetting.end()) {
                Log("SaveGame: character not found: " + name, __func__, Logcode::NOTFOUND);
                return nullptr;
            }

            auto character = std::make_unique<CharacterObject>();
            character->NewCharacter(name);
            character->game_attribute.stars = j.value("stars", 1);

            if (j.contains("equipment") && j["equipment"].is_array()) {
                for (const auto& ej : j["equipment"]) {
                    auto equipment = LoadEquipment(ej);
                    if (!equipment) continue;
                    if (character->GetEquipments().size() >= CharacterObject::EquipmentSlotLimit) break;
                    character->GetEquipments().push_back(std::move(equipment));
                }
            }
            return character;
        }

        json SaveMineral(const Mineral& mineral) {
            json j;
            j["quality"] = MineralTypeName(mineral.Quality());
            j["specified"] = mineral.IsSpecified();
            j["money"] = mineral.GetMoney();
            j["exp"] = mineral.GetExp();

            j["characters"] = json::array();
            for (const auto& character : mineral.GetCharacter()) {
                if (character) j["characters"].push_back(SaveCharacter(*character));
            }
            j["equipments"] = json::array();
            for (const auto& equipment : mineral.GetEquipment()) {
                if (equipment) j["equipments"].push_back(SaveEquipment(*equipment));
            }
            return j;
        }

        std::unique_ptr<Mineral> LoadMineral(const json& j) {
            if (!j.is_object()) return nullptr;
            const MineralType quality = ParseMineralType(j.value("quality", std::string("Grey")));
            auto mineral = std::make_unique<Mineral>(quality);
            mineral->SetSpecified(j.value("specified", false));
            mineral->SetMoney(j.value("money", 0));
            mineral->SetExp(j.value("exp", 0));

            if (j.contains("characters") && j["characters"].is_array()) {
                for (const auto& cj : j["characters"]) {
                    auto character = LoadCharacter(cj);
                    if (character) mineral->GetCharacter().push_back(std::move(character));
                }
            }
            if (j.contains("equipments") && j["equipments"].is_array()) {
                for (const auto& ej : j["equipments"]) {
                    auto equipment = LoadEquipment(ej);
                    if (equipment) mineral->GetEquipment().push_back(std::move(equipment));
                }
            }
            return mineral;
        }

        struct ZoneDef {
            Logic::OriginPlaceType type;
            const char* name;
        };

        const ZoneDef kZones[] = {
            { Logic::OriginPlaceType::FrontBattleField, "FrontBattleField" },
            { Logic::OriginPlaceType::BackBattleField, "BackBattleField" },
            { Logic::OriginPlaceType::Bench, "Bench" },
            { Logic::OriginPlaceType::ShopCharacter, "ShopCharacter" },
            { Logic::OriginPlaceType::ConsumableEquipment, "ConsumableEquipment" },
            { Logic::OriginPlaceType::Equipment, "Equipment" },
            { Logic::OriginPlaceType::Mineral, "Mineral" },
        };

        json SaveZones(const Logic& logic) {
            json zones = json::object();
            for (const auto& def : kZones) {
                json zone;
                zone["places"] = json::array();
                for (const auto& rect : logic.ObjectPlace(def.type)) {
                    zone["places"].push_back(json::array({ rect.x, rect.y, rect.w, rect.h }));
                }

                zone["slots"] = json::array();
                for (const auto& object : logic.Zone(def.type)) {
                    if (!object) {
                        zone["slots"].push_back(nullptr);
                        continue;
                    }
                    json entry;
                    if (auto* character = dynamic_cast<CharacterObject*>(object.get())) {
                        entry["type"] = "Character";
                        entry["data"] = SaveCharacter(*character);
                    }
                    else if (auto* equipment = dynamic_cast<BasicEquipment*>(object.get())) {
                        entry["type"] = "Equipment";
                        entry["data"] = SaveEquipment(*equipment);
                    }
                    else if (auto* mineral = dynamic_cast<Mineral*>(object.get())) {
                        entry["type"] = "Mineral";
                        entry["data"] = SaveMineral(*mineral);
                    }
                    else {
                        zone["slots"].push_back(nullptr);
                        continue;
                    }
                    zone["slots"].push_back(std::move(entry));
                }
                zones[def.name] = std::move(zone);
            }
            return zones;
        }

        void LoadZones(Logic& logic, const json& zones) {
            if (!zones.is_object()) return;
            for (const auto& def : kZones) {
                auto it = zones.find(def.name);
                if (it == zones.end() || !it->is_object()) continue;
                const json& zoneJson = *it;

                auto& zone = logic.Zone(def.type);
                auto& places = logic.ObjectPlace(def.type);
                zone.clear();
                places.clear();

                if (zoneJson.contains("places") && zoneJson["places"].is_array()) {
                    for (const auto& rj : zoneJson["places"]) {
                        if (!rj.is_array() || rj.size() < 4) continue;
                        places.push_back(SDL_FRect{
                            rj[0].get<float>(), rj[1].get<float>(),
                            rj[2].get<float>(), rj[3].get<float>() });
                    }
                }

                if (!zoneJson.contains("slots") || !zoneJson["slots"].is_array()) {
                    continue;
                }
                const auto& slots = zoneJson["slots"];
                zone.resize(slots.size());
                for (size_t i = 0; i < slots.size(); ++i) {
                    const json& slot = slots[i];
                    if (!slot.is_object()) continue;
                    const std::string type = slot.value("type", std::string(""));
                    if (!slot.contains("data")) continue;
                    const json& data = slot["data"];

                    if (type == "Character") {
                        zone[i] = LoadCharacter(data);
                    }
                    else if (type == "Equipment") {
                        zone[i] = LoadEquipment(data);
                    }
                    else if (type == "Mineral") {
                        zone[i] = LoadMineral(data);
                    }
                }
                if (places.size() < zone.size()) {
                    places.resize(zone.size(), SDL_FRect{ 0.0f, 0.0f, 0.0f, 0.0f });
                }
            }
        }

        json SaveShop(const Shop& shop) {
            json j;
            j["totalExp"] = shop.TotalExp;
            j["level"] = shop.ShopLevel;
            j["dcardMoney"] = shop.Dcardmoney;
            j["freeDcard"] = shop.FreeDcardTimes;
            j["buyExpMoney"] = shop.BuyExpmoney;
            j["expBuyNum"] = shop.ExpBuyNum;
            j["defaultLimit"] = shop.defaultlimit;
            j["potential"] = shop.Potential;
            j["expLevelUp"] = shop.explevelup;

            j["pool"] = json::array();
            for (const auto& costPool : shop.ShopPool) {
                json counts = json::object();
                for (const auto& [name, vec] : costPool) {
                    counts[name] = static_cast<int>(vec.size());
                }
                j["pool"].push_back(std::move(counts));
            }

            j["limits"] = json::array();
            for (const auto& limits : shop.ShopCharacterNumLimit) {
                j["limits"].push_back(limits);
            }

            j["realShop"] = json::array();
            for (const auto& card : shop.RealShop) {
                if (!card) {
                    j["realShop"].push_back(nullptr);
                    continue;
                }
                auto* character = dynamic_cast<CharacterObject*>(card.get());
                if (!character) {
                    j["realShop"].push_back(nullptr);
                    continue;
                }
                j["realShop"].push_back(SaveCharacter(*character));
            }
            return j;
        }

        void LoadShop(Logic& logic, const std::filesystem::path& gamePath, const json& j) {
            Shop& shop = logic.shop;
            shop.LoadinJson(gamePath);

            if (!j.is_object()) return;
            shop.TotalExp = j.value("totalExp", shop.TotalExp);
            shop.ShopLevel = j.value("level", shop.ShopLevel);
            shop.Dcardmoney = j.value("dcardMoney", shop.Dcardmoney);
            shop.FreeDcardTimes = j.value("freeDcard", shop.FreeDcardTimes);
            shop.BuyExpmoney = j.value("buyExpMoney", shop.BuyExpmoney);
            shop.ExpBuyNum = j.value("expBuyNum", shop.ExpBuyNum);
            shop.defaultlimit = j.value("defaultLimit", shop.defaultlimit);

            try {
                if (j.contains("potential")) shop.Potential = j["potential"].get<std::vector<std::vector<double>>>();
                if (j.contains("expLevelUp")) shop.explevelup = j["expLevelUp"].get<std::vector<int>>();
            }
            catch (const std::exception& e) {
                Log(std::string("SaveGame shop config: ") + e.what(), __func__, Logcode::ERROR);
            }

            if (shop.ShopPool.size() < 5) {
                Log("SaveGame: shop pool not initialized", __func__, Logcode::ERROR);
                return;
            }
            if (shop.ShopCharacterNumLimit.size() < 5) shop.ShopCharacterNumLimit.resize(5);

            if (j.contains("pool") && j["pool"].is_array()) {
                const auto& poolJson = j["pool"];
                for (size_t cost = 0; cost < poolJson.size() && cost < shop.ShopPool.size(); ++cost) {
                    if (!poolJson[cost].is_object()) continue;
                    for (auto it = poolJson[cost].begin(); it != poolJson[cost].end(); ++it) {
                        const std::string name = it.key();
                        const int count = it.value().get<int>();
                        auto& vec = shop.ShopPool[cost][name];
                        if (static_cast<int>(vec.size()) > count) {
                            vec.resize(static_cast<size_t>(count));
                        }
                        while (static_cast<int>(vec.size()) < count) {
                            auto character = std::make_unique<CharacterObject>();
                            character->NewCharacter(name);
                            vec.push_back(std::move(character));
                        }
                    }
                }
            }

            if (j.contains("limits") && j["limits"].is_array()) {
                const auto& limitsJson = j["limits"];
                for (size_t cost = 0; cost < limitsJson.size() && cost < shop.ShopCharacterNumLimit.size(); ++cost) {
                    if (!limitsJson[cost].is_object()) continue;
                    for (auto it = limitsJson[cost].begin(); it != limitsJson[cost].end(); ++it) {
                        shop.ShopCharacterNumLimit[cost][it.key()] = it.value().get<int>();
                    }
                }
            }

            shop.RealShop.clear();
            if (j.contains("realShop") && j["realShop"].is_array()) {
                for (const auto& cj : j["realShop"]) {
                    if (!cj.is_object()) {
                        shop.RealShop.push_back(nullptr);
                        continue;
                    }
                    auto character = LoadCharacter(cj);
                    shop.RealShop.push_back(std::move(character));
                }
            }
            while (shop.RealShop.size() < 5) shop.RealShop.push_back(nullptr);
        }

    } // namespace

    std::filesystem::path SavePath(const std::filesystem::path& gamePath) {
        return gamePath / "save" / "run.json";
    }

    bool HasSave(const std::filesystem::path& gamePath) {
        std::error_code ec;
        return std::filesystem::exists(SavePath(gamePath), ec);
    }

    bool Delete(const std::filesystem::path& gamePath) {
        std::error_code ec;
        std::filesystem::remove(SavePath(gamePath), ec);
        return !ec;
    }

    bool PeekMeta(const std::filesystem::path& gamePath,
        std::vector<std::string>& bossNames,
        int& difficulty) {
        std::ifstream in(SavePath(gamePath));
        if (!in.is_open()) return false;

        json j;
        try {
            in >> j;
        }
        catch (const std::exception& e) {
            Log(std::string("SaveGame peek: ") + e.what(), __func__, Logcode::PARSEERROR);
            return false;
        }

        if (j.value("version", 0) != 1) return false;
        if (!j.contains("bossNames") || !j["bossNames"].is_array()) return false;
        bossNames = j["bossNames"].get<std::vector<std::string>>();
        difficulty = j.value("difficulty", 137);
        if (bossNames.empty()) return false;

        StageData::g_GameStart_WaitingForFight.Boss_name = bossNames;
        StageData::g_GameStart_WaitingForFight.difficulty = difficulty;
        GameStart::State.Boss_name = bossNames;
        GameStart::State.difficulty = difficulty;
        return true;
    }

    bool Save(const Logic& logic, const std::filesystem::path& gamePath) {
        json root;
        root["version"] = 1;
        root["dimension"] = logic.dimension_index;
        root["stage"] = logic.stage_index;
        root["blood"] = logic.Blood;
        root["money"] = logic.MoneyNum;
        root["maxInterest"] = logic.MaxInterest;
        root["continueWin"] = logic.ContinueWinTimes;
        root["difficulty"] = logic.Difficulty;
        root["battlefieldLimit"] = logic.CharacterOnBattleFieldLimit;
        root["benchLimit"] = logic.CharacterOnBenchLimit;
        root["encounterBonus"] = logic.encounter_difficulty_bonus;

        root["bossNames"] = json::array();
        if (!logic.boss_name.empty()) {
            for (const auto& name : logic.boss_name) root["bossNames"].push_back(name);
        }
        else {
            for (const auto& boss : logic.Boss) root["bossNames"].push_back(boss.name);
        }

        root["enemies"] = json::array();
        for (const auto& enemy : logic.Enemys) root["enemies"].push_back(enemy.name);

        root["environment"] = GameStart::State.environment_acquired;
        root["permanentStrategies"] = json::array();
        for (const auto& strategy : Strategy::Bus::Instance().SavePermanentStates()) {
            root["permanentStrategies"].push_back(json{
                {"id", strategy.id},
                {"state", strategy.state}
                });
        }
        root["stageStrategies"] = logic.stage_strategies;
        root["stageStrategySelected"] = logic.stage_strategy_selected;
        root["stageStrategyRefresh"] = json::array({
            logic.stage_strategy_refresh_left[0],
            logic.stage_strategy_refresh_left[1],
            logic.stage_strategy_refresh_left[2] });
        root["supplyCharacters"] = logic.supply_characters;
        root["supplyEquipments"] = logic.supply_equipments;
        root["supplySelected"] = logic.supply_selected;
        root["supplyRefresh"] = logic.supply_refresh_left;
        root["encounterHardness"] = json::array({ logic.encounter_hardness[0], logic.encounter_hardness[1] });
        root["encounterSelected"] = logic.encounter_selected;
        root["encounterRefresh"] = logic.encounter_refresh_left;
        root["zones"] = SaveZones(logic);
        root["shop"] = SaveShop(logic.shop);

        const std::filesystem::path path = SavePath(gamePath);
        std::error_code ec;
        std::filesystem::create_directories(path.parent_path(), ec);
        if (ec) {
            Log("SaveGame: create save dir failed", __func__, Logcode::CREATEFAILED);
            return false;
        }

        const std::filesystem::path tmp = path.string() + ".tmp";
        std::ofstream out(tmp, std::ios::trunc);
        if (!out.is_open()) {
            Log("SaveGame: open tmp failed", __func__, Logcode::CREATEFAILED);
            return false;
        }
        out << root.dump(2);
        out.close();

        std::filesystem::remove(path, ec);
        std::filesystem::rename(tmp, path, ec);
        if (ec) {
            Log("SaveGame: rename failed", __func__, Logcode::ERROR);
            std::filesystem::remove(tmp, ec);
            return false;
        }
        Log("SaveGame: saved " + path.string(), __func__, Logcode::OK);
        return true;
    }

    bool Load(Logic& logic, const std::filesystem::path& gamePath) {
        std::ifstream in(SavePath(gamePath));
        if (!in.is_open()) {
            Log("SaveGame: save file missing", __func__, Logcode::OPENFAILED);
            return false;
        }

        json root;
        try {
            in >> root;
        }
        catch (const std::exception& e) {
            Log(std::string("SaveGame: parse failed: ") + e.what(), __func__, Logcode::PARSEERROR);
            return false;
        }

        const int version = root.value("version", 0);
        if (version != 1) {
            Log("SaveGame: unsupported version", __func__, Logcode::ERROR);
            return false;
        }

        const size_t dimension = root.value("dimension", static_cast<size_t>(0));
        const size_t stage = root.value("stage", static_cast<size_t>(0));
        if (dimension >= logic.stage.size() ||
            stage >= logic.stage[dimension].size()) {
            Log("SaveGame: invalid stage index", __func__, Logcode::ERROR);
            return false;
        }

        logic.dimension_index = dimension;
        logic.stage_index = stage;
        logic.Blood = root.value("blood", logic.Blood);
        logic.MoneyNum = root.value("money", logic.MoneyNum);
        logic.MaxInterest = root.value("maxInterest", logic.MaxInterest);
        logic.ContinueWinTimes = root.value("continueWin", logic.ContinueWinTimes);
        logic.Difficulty = root.value("difficulty", logic.Difficulty);
        logic.CharacterOnBattleFieldLimit = root.value("battlefieldLimit", logic.CharacterOnBattleFieldLimit);
        logic.CharacterOnBenchLimit = root.value("benchLimit", logic.CharacterOnBenchLimit);
        logic.encounter_difficulty_bonus = root.value("encounterBonus", 0);
        logic.last_dimension = static_cast<int>(logic.dimension_index);
        logic.last_continue_win_times = logic.ContinueWinTimes;

        if (root.contains("bossNames") && root["bossNames"].is_array()) {
            logic.boss_name = root["bossNames"].get<std::vector<std::string>>();
        }
        if (!logic.boss_name.empty()) {
            Object::Enemy_LoadinJson(gamePath, logic.Boss, logic.boss_name);
            StageData::g_GameStart_WaitingForFight.Boss_name = logic.boss_name;
            StageData::g_GameStart_WaitingForFight.difficulty = logic.Difficulty;
            GameStart::State.Boss_name = logic.boss_name;
            GameStart::State.difficulty = logic.Difficulty;
        }

        logic.Enemys.clear();
        if (root.contains("enemies") && root["enemies"].is_array()) {
            for (const auto& name : root["enemies"].get<std::vector<std::string>>()) {
                auto it = Object::Enemy_DefaultSetting.find(name);
                if (it == Object::Enemy_DefaultSetting.end()) {
                    Log("SaveGame: enemy not found: " + name, __func__, Logcode::NOTFOUND);
                    continue;
                }
                logic.Enemys.push_back(it->second);
            }
        }

        if (root.contains("environment") && root["environment"].is_array()) {
            GameStart::State.environment_acquired =
                root["environment"].get<std::vector<std::string>>();
        }
        Strategy::Registry::Instance().LoadPool(gamePath);
        if (root.contains("permanentStrategies") && root["permanentStrategies"].is_array()) {
            std::vector<Strategy::PermanentState> states;
            for (const auto& item : root["permanentStrategies"]) {
                if (!item.is_object()) continue;
                Strategy::PermanentState state;
                state.id = item.value("id", std::string(""));
                state.state = item.value("state", json::object());
                if (!state.id.empty()) states.push_back(std::move(state));
            }
            Strategy::Bus::Instance().LoadPermanentStates(states);
        }
        else {
            // older save file: only environment IDs were stored
            Strategy::Bus::Instance().ClearPermanentStrategies();
            for (const auto& id : GameStart::State.environment_acquired) {
                Strategy::Bus::Instance().AddPermanentStrategy(id);
            }
        }

        if (root.contains("stageStrategies") && root["stageStrategies"].is_array()) {
            logic.stage_strategies = root["stageStrategies"].get<std::vector<std::string>>();
        }
        logic.stage_strategy_selected = root.value("stageStrategySelected", -1);
        if (root.contains("stageStrategyRefresh") && root["stageStrategyRefresh"].is_array()) {
            const auto& arr = root["stageStrategyRefresh"];
            for (size_t i = 0; i < logic.stage_strategy_refresh_left.size() && i < arr.size(); ++i) {
                logic.stage_strategy_refresh_left[i] = arr[i].get<int>();
            }
        }
        if (root.contains("supplyCharacters") && root["supplyCharacters"].is_array()) {
            const auto& arr = root["supplyCharacters"];
            for (size_t i = 0; i < logic.supply_characters.size() && i < arr.size(); ++i) {
                logic.supply_characters[i] = arr[i].get<std::string>();
            }
        }
        if (root.contains("supplyEquipments") && root["supplyEquipments"].is_array()) {
            const auto& arr = root["supplyEquipments"];
            for (size_t i = 0; i < logic.supply_equipments.size() && i < arr.size(); ++i) {
                logic.supply_equipments[i] = arr[i].get<std::string>();
            }
        }
        logic.supply_selected = root.value("supplySelected", -1);
        logic.supply_refresh_left = root.value("supplyRefresh", 1);
        if (root.contains("encounterHardness") && root["encounterHardness"].is_array()) {
            const auto& arr = root["encounterHardness"];
            for (size_t i = 0; i < logic.encounter_hardness.size() && i < arr.size(); ++i) {
                logic.encounter_hardness[i] = arr[i].get<int>();
            }
        }
        logic.encounter_selected = root.value("encounterSelected", -1);
        logic.encounter_refresh_left = root.value("encounterRefresh", 1);

        if (root.contains("zones")) LoadZones(logic, root["zones"]);
        if (root.contains("shop")) LoadShop(logic, gamePath, root["shop"]);

        logic.holding.reset();
        logic.Mmode = Logic::MouseMode::Idle;
        logic.LeftButtonDown = false;
        logic.Rmode = Logic::RealTimeMode::Normal;
        logic.AtWaitingForFight = true;
        return true;
    }

}
