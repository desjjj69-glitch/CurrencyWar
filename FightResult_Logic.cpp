#include"FightResult_Logic.h"
#include"StageData.h"
#include"SDL_Functions.h"
#include"BasicGameResource.h"
#include<fstream>
#include<sstream>
#include"json.hpp"
#include"Logger.h"
using json = nlohmann::json;
using Logger::Log;
namespace FightResult{
	static bool PlaceInside(float x, float y, SDL_FRect rect) {
		return x >= rect.x && x <= rect.x + rect.w && y >= rect.y && y <= rect.y + rect.h;
	}
	SDL_FRect Logic::GetUIRect(
		const std::string& stringname,
		std::size_t index)
	{
		auto it = ui_place.find(stringname);
		if (it == ui_place.end())          // key 不存在
			return SDL_FRect{ -0.f, 0.f, 0.f, 0.f };

		const auto& vec = it->second;
		if (index >= vec.size())           // index 越界 / 为空
			return SDL_FRect{ 0.f, 0.f, 0.f, 0.f };

		return vec[index];
	}
	static bool empty(SDL_FRect rect) {
		return rect.x == 0 &&
			rect.y == 0 &&
			rect.w == 0 &&
			rect.h == 0;
	}
}
namespace FightResult {
	Logic State;
	
	void Logic::Handle() {
		
		using StageData::g_Fight_to_FightResult;
		using StageData::g_WaitingForFight_to_FightResult;
		using StageData::g_WaitingForFight_publicData;
		auto& r = g_WaitingForFight_to_FightResult;

		if (!r.money || !r.interest_limit || !r.ContinueWinTimes
			|| !r.Blood || !r.ExpNum) {
			Log("FightResult pointer null", __FUNCSIG__, Logcode::ERROR);
			return;
		}
		if (g_WaitingForFight_to_FightResult.ContinueWinTimes) {//处理连胜
			const bool isBonus =
				(StageData::g_WaitingForFight_to_Fight.stagetype == "Bonus");
			if (g_Fight_to_FightResult.ifwin) {
				if (!isBonus) {
					++(*g_WaitingForFight_to_FightResult.ContinueWinTimes);
				}
			}
			else
				*g_WaitingForFight_to_FightResult.ContinueWinTimes = 0;
		}
		HandleMoney(
			*(g_WaitingForFight_to_FightResult.money),
			*(g_WaitingForFight_to_FightResult.interest_limit),
			*(g_WaitingForFight_to_FightResult.ContinueWinTimes),
			*(g_WaitingForFight_publicData.dimension_index),
			*(g_WaitingForFight_publicData.stage_index)
		);
		HandleExp(
			*(g_WaitingForFight_to_FightResult.ExpNum),
			*(g_WaitingForFight_publicData.dimension_index),
			*(g_WaitingForFight_publicData.stage_index)
		);
		HandleBlood(
			*(g_WaitingForFight_to_FightResult.Blood),
			*(g_WaitingForFight_publicData.dimension_index),
			*(g_WaitingForFight_publicData.stage_index)
		);
		//最后再阶段+1
		++(*StageData::g_WaitingForFight_publicData.stage_index);//阶段+1，如果没有blood归零的话
	}
	void Logic::EventLogic() {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			SDL_ConvertEventToRenderCoordinates(BasicGameRes.GameRenderer, &event);
			switch (event.type) {
			case SDL_EVENT_KEY_DOWN:
				switch (event.key.scancode) {
				//case SDL_SCANCODE_ESCAPE:
				//	AtFightResult = false;
				//	break;
				}
				break;
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
			{
				auto rect = GetUIRect("quit", 0);
				if (empty(rect))break;
				if(PlaceInside(event.button.x,event.button.y,rect))
				AtFightResult = false;
				break;
			}
				
				
			}
		}
	}
	void Logic::LoadinJson(const std::filesystem::path& gp) {
		std::ifstream configin(gp / "config" / "FightResult.json");
		json js;
		Log("Result", __FUNCSIG__, configin.is_open() ? Logcode::OK : Logcode::OPENFAILED);
		if (!configin.is_open())return;
		
		try {
			configin >> js;
			if (js.contains("Money") && js["Money"].is_array()) {
				for (auto& var : js["Money"]) {
					if (var.is_object() && var.contains("Basic")) {
						BasicMoneyGain = var.at("Basic").get < std::vector<std::vector<int>>>();
					}
					else
						if (var.is_object() && var.contains("ContinueWin")) {
							ContinueWinMoneyGain = var.at("ContinueWin").get<std::vector<int>>();
						}
				}
			}
			if (js.contains("Exp") && js["Exp"].is_array())ExpGain = js.at("Exp").get<std::vector<std::vector<int>>>();
			if (js.contains("Blood") && js["Blood"].is_array())BloodGain = js.at("Blood").get<std::vector<std::vector<int>>>();

		}
		catch (const json::parse_error& e) {
			Log(e.what(), __FUNCSIG__, Logcode::PARSEERROR);
		}
		catch (const std::exception& e) {
			Log(e.what(), __FUNCSIG__, Logcode::ERROR);
		}
		auto ParseRect = [](const std::string& s) ->SDL_FRect{
			std::istringstream iss(s);
			SDL_FRect rect{ 0,0,0,0 };
			iss >> rect.x >> rect.y >> rect.w >> rect.h;
			if (iss.fail())return { 0,0,0,0 };
			return rect;
			};
		std::ifstream uiin(gp / "data" / "FightResult_Texture.json");
		json uijs;
		try {
			uiin >> uijs;
			for (auto& [key, vec] : uijs.items()) {
				if (vec.is_array()) {
					for (auto& var : vec) {
						if (var.contains("place") && var["place"].is_string()) {
							ui_place[key].push_back(ParseRect(var["place"].get<std::string>()));
						}
						else {
							ui_place[key].push_back({ 0,0,0,0 });
						}
					}
				}
			}
		}
		catch (const json::parse_error& e) {
			Log(e.what(), __FUNCSIG__, Logcode::PARSEERROR);
		}
		catch (const std::exception& e) {
			Log(e.what(), __FUNCSIG__, Logcode::ERROR);
		}
	}
	void Logic::HandleMoney(int& money,int max_interest,int continuewin, size_t dimension, size_t index) {
		money += (std::min(max_interest, money / 10));//利息
		if (!ContinueWinMoneyGain.empty()) {
			if (continuewin >= ContinueWinMoneyGain.size()) {
				money += ContinueWinMoneyGain[ContinueWinMoneyGain.size() - 1];
			}
			else {
				money += ContinueWinMoneyGain[continuewin];
			}
		}//连胜
		if (dimension < BasicMoneyGain.size()) {
			if (index < BasicMoneyGain[dimension].size()) {
				money += BasicMoneyGain[dimension][index];
			}
		}//基础
	}
	void Logic::HandleBlood(int& blood, size_t dimension, size_t index) {
		if (dimension < BloodGain.size()&&index < BloodGain[dimension].size()) {
			
				blood += BloodGain[dimension][index];
			
		}
		else {
			if (StageData::g_Fight_to_FightResult.ifwin) {
				blood += BloodGain[3][0];
			}
		}
		
	}
	void Logic::HandleExp(int& exp, size_t dimension, size_t index) {
		if (dimension < ExpGain.size()) {
			if (index < ExpGain[dimension].size()) {
				exp += ExpGain[dimension][index];
			}
		}
	}
}