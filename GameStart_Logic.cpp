#include"GameStart_Logic.h"
#include<SDL3/SDL.h>
#include"Logger.h"
#include"BasicGameResource.h"
#include"json.hpp"
#include"GetRandomNum.h"
#include"StageData.h"
#include<fstream>
#include<sstream>
using json = nlohmann::json;
using Logger::Log;
namespace GameStart {
    Logic State;
	void Logic::Init() {
        state = State::Waiting;
		LoadinFromJson(BasicGameRes.GamePath);
        using StageData::g_GameStart_WaitingForFight;
        g_GameStart_WaitingForFight.Boss_name = Boss_name;
        g_GameStart_WaitingForFight.difficulty = difficulty;
        Init_Minerals();
	}
    void Logic::Init_Minerals() {
        using StageData::g_GameStart_WaitingForFight;
        g_GameStart_WaitingForFight.minerals.clear();

        //要在开局就确定具体角色，先保证角色默认表已经读入
        Object::Character_LoadinJson(BasicGameRes.GamePath);
        auto PickByCost = [](int cost) -> std::string {
            std::vector<std::string> pool;
            for (auto& [name, ch] : Object::Character_DefaultSetting) {
                if (ch.game_attribute.cost == cost) pool.push_back(name);
            }
            if (pool.empty()) return {};
            return pool[static_cast<size_t>(GetRandomNum(0, static_cast<int>(pool.size()) - 1))];
            };

        StageData::MineralStartData grey_money;
        grey_money.quality = MineralType::Grey;
        grey_money.money = 1;

        StageData::MineralStartData grey_character;
        grey_character.quality = MineralType::Grey;
        if (auto name = PickByCost(1); !name.empty()) {
            grey_character.characters.push_back(std::move(name));
        }

        StageData::MineralStartData blue_character;
        blue_character.quality = MineralType::Blue;
        if (auto name = PickByCost(2); !name.empty()) {
            blue_character.characters.push_back(std::move(name));
        }

        g_GameStart_WaitingForFight.minerals.push_back(std::move(grey_money));
        g_GameStart_WaitingForFight.minerals.push_back(std::move(grey_character));
        g_GameStart_WaitingForFight.minerals.push_back(std::move(blue_character));

        Log("Mineral: 2 Grey + 1 Blue", __func__, Logcode::OK);
    }
	void Logic::LoadinFromJson(const std::filesystem::path& gp) {
		Boss_name.clear();
        Boss_name.resize(3);
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

                Boss_name[i] = bossattr.at("name").get<std::string>();

                

                
            }
        }

        std::ifstream gameresource(gp / "config" / "GameResource.json");
        json gr_js;
        try {
            gameresource >> gr_js;
        }
        catch (const json::parse_error& e) {
            Log(e.what(), __func__, Logcode::PARSEERROR);
        }
        catch (const std::exception& e) {
            Log(e.what(), __func__, Logcode::ERROR);
        }
        if (gr_js.contains("Difficulty") && gr_js["Difficulty"].is_number_integer()) {
            difficulty = gr_js["Difficulty"].get<int>();
        }

        // ==================== 2. 读入 UI 位置，供点击判定使用 ====================
        // 与 GameStart_Texture.json 的 place 字段保持一致
        ui_place.clear();
        auto ParseRect = [](const std::string& s) -> SDL_FRect {
            std::istringstream iss(s);
            SDL_FRect rect{ 0,0,0,0 };
            iss >> rect.x >> rect.y >> rect.w >> rect.h;
            if (iss.fail()) return { 0,0,0,0 };
            return rect;
            };
        std::ifstream uiin(gp / "data" / "GameStart_Texture.json");
        if (!uiin.is_open()) {
            Log("Cannot open data/GameStart_Texture.json", __func__, Logcode::OPENFAILED);
        }
        else {
            json uijs;
            try {
                uiin >> uijs;
                for (auto& [key, obj] : uijs.items()) {
                    if (!obj.is_object() || !obj.contains("place")) continue;
                    const json& place = obj["place"];
                    if (place.is_array()) {
                        for (auto& var : place) {
                            if (var.is_string())
                                ui_place[key].push_back(ParseRect(var.get<std::string>()));
                            else
                                ui_place[key].push_back({ 0,0,0,0 });
                        }
                    }
                    else if (place.is_string()) {
                        ui_place[key].push_back(ParseRect(place.get<std::string>()));
                    }
                    else {
                        ui_place[key].push_back({ 0,0,0,0 });
                    }
                }
            }
            catch (const json::parse_error& e) {
                Log(e.what(), __func__, Logcode::PARSEERROR);
            }
            catch (const std::exception& e) {
                Log(e.what(), __func__, Logcode::ERROR);
            }
        }
	}
    void Logic::EventLogic() {
        SDL_Event event;
            while (SDL_PollEvent(&event)) {
                SDL_ConvertEventToRenderCoordinates(BasicGameRes.GameRenderer, &event);
                switch (event.type) {
                case SDL_EVENT_KEY_DOWN:
                {
                    switch (event.key.scancode) {
                    case SDL_SCANCODE_SPACE:
                        state = State::Continue;
                        return;
                    }
                }
                break;
                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                {
                    if (event.button.button != SDL_BUTTON_LEFT) break;
                    auto it = ui_place.find("nextstep");
                    if (it == ui_place.end() || it->second.empty()) break;
                    if (PlaceInside(event.button.x, event.button.y, it->second.front())) {
                        state = State::Continue;
                        return;
                    }
                    break;
                }
                }
            }
        
    }
}