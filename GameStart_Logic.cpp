#include"GameStart_Logic.h"
#include<SDL3/SDL.h>
#include"Logger.h"
#include"BasicGameResource.h"
#include"json.hpp"
#include"GetRandomNum.h"
#include"StageData.h"
#include"Strategy/StrategyRegistry.h"
#include"Strategy/StrategyBus.h"
#include<fstream>
#include<sstream>
using json = nlohmann::json;
using Logger::Log;
namespace GameStart {
    Logic State;
	void Logic::Init() {
        state = State::Waiting;
        environment_strategies.clear();
        environment_acquired.clear();
        environment_selected = -1;
        environment_refresh_left = 1;
        Strategy::Registry::Instance().LoadPool(BasicGameRes.GamePath);//完成策略的注册
		LoadinFromJson(BasicGameRes.GamePath);
        using StageData::g_GameStart_WaitingForFight;
        g_GameStart_WaitingForFight.Boss_name = Boss_name;
        g_GameStart_WaitingForFight.difficulty = difficulty;
        g_GameStart_WaitingForFight.money_bonus = 0;
	}
    void Logic::BeginEnvironmentChoose() {
        environment_selected = -1;
        environment_refresh_left = 1;
        environment_strategies = Strategy::Registry::Instance().PickRandom(3, environment_acquired, "EnvironmentStrategy");
        Log("EnvironmentChoose pick=" + std::to_string(environment_strategies.size()), __func__, Logcode::OK);
    }
    void Logic::RefreshEnvironment() {
        if (environment_refresh_left <= 0) {
            return;
        }
        --environment_refresh_left;
        environment_selected = -1;
        environment_strategies = Strategy::Registry::Instance().PickRandom(3, environment_acquired, "EnvironmentStrategy");
        Log("EnvironmentChoose refresh=" + std::to_string(environment_strategies.size()), __func__, Logcode::OK);
    }
    void Logic::ConfirmEnvironment(int index) {
        if (index < 0 || index >= static_cast<int>(environment_strategies.size())) {
            return;
        }
        const std::string& id = environment_strategies[static_cast<size_t>(index)];
        if (id.empty()) {
            return;
        }
        Strategy::Bus::Instance().AddPermanentStrategy(id);
        environment_acquired.push_back(id);
        Log("EnvironmentChoose confirm=" + id, __func__, Logcode::OK);
        if (id == "Strategy_13") {
            //Strategy_13：立刻记 6 金币，然后重抽一次环境，且不再提供刷新
            StageData::g_GameStart_WaitingForFight.money_bonus += 6;
            environment_selected = -1;
            environment_refresh_left = 0;
            environment_strategies = Strategy::Registry::Instance().PickRandom(3, environment_acquired, "EnvironmentStrategy");
            Log("EnvironmentChoose redraw after Strategy_13", __func__, Logcode::OK);
            return;
        }
        state = State::Continue;
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
                switch (state) {
                case State::Waiting: {
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
                            state = State::EnvironmentChoose;
                            BeginEnvironmentChoose();
                            return;
                        }
                        break;
                    }
                    }
                }break;
                case State::EnvironmentChoose: {
                    switch (event.type) {
                    case SDL_EVENT_MOUSE_BUTTON_DOWN:
                    {
                        if (event.button.button != SDL_BUTTON_LEFT) break;

                        //确认按钮：未选择时点击无反应
                        auto confirmIt = ui_place.find("confirm");
                        if (confirmIt != ui_place.end() && !confirmIt->second.empty() &&
                            PlaceInside(event.button.x, event.button.y, confirmIt->second[0])) {
                            if (environment_selected >= 0) {
                                ConfirmEnvironment(environment_selected);
                            }
                            return;
                        }

                        //刷新按钮：默认一次，用完后按钮不再绘制
                        auto refreshIt = ui_place.find("refresh");
                        if (refreshIt != ui_place.end() && !refreshIt->second.empty() &&
                            PlaceInside(event.button.x, event.button.y, refreshIt->second[0])) {
                            RefreshEnvironment();
                            return;
                        }

                        //策略卡：只负责选中，不再通过再次点击同一格确认
                        auto it = ui_place.find("environment");
                        if (it == ui_place.end()) break;
                        for (size_t i = 0; i < it->second.size() && i < environment_strategies.size(); ++i) {
                            if (!PlaceInside(event.button.x, event.button.y, it->second[i])) continue;
                            environment_selected = static_cast<int>(i);
                            return;
                        }
                        break;
                    }
                    case SDL_EVENT_KEY_DOWN:
                        if (event.key.scancode == SDL_SCANCODE_SPACE && environment_selected >= 0) {
                            ConfirmEnvironment(environment_selected);
                            return;
                        }
                        break;
                    }
                }break;
                }
                
            }
        
    }
}