#include"GameStart_Logic.h"
#include<SDL3/SDL.h>
#include"Logger.h"
#include"BasicGameResource.h"
#include"json.hpp"
#include"GetRandomNum.h"
#include"StageData.h"
using json = nlohmann::json;
using Logger::Log;
namespace GameStart {
    Logic State;
	void Logic::Init() {
        state = State::Waiting;
		LoadinFromJson(BasicGameRes.GamePath);
        using StageData::g_GameStart_WaitingForFight;
        g_GameStart_WaitingForFight.Boss_name = Boss_name;
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
	}
    void Logic::EventLogic() {
        SDL_Event event;
            while (SDL_PollEvent(&event)) {
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
                }
            }
        
    }
}