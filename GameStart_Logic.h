#pragma once
#include<SDL3/SDL.h>
#include<filesystem>
#include<string>
#include<unordered_map>
#include<vector>
#include"SDL_Functions.h"
namespace GameStart {
	class Logic {
	public:
		std::vector<std::string> Boss_name;
		std::unordered_map<std::string, std::vector<SDL_FRect>> ui_place;
		int difficulty = 137;
		std::vector<std::string> environment_strategies;//本局环境选择抽到的三个策略 id
		int environment_selected = -1;//当前选中的环境/策略下标
		void Init();

		void EventLogic();
		enum class State {
			Continue,
			Waiting,
			EnvironmentChoose
		};
		State state = State::Waiting;
	private:
		static bool PlaceInside(float x, float y, const SDL_FRect& rect) {
			return x >= rect.x && x <= rect.x + rect.w && y >= rect.y && y <= rect.y + rect.h;
		}
		void Init_Minerals();//开局固定 2 灰 + 1 蓝，写入交接数据
		void BeginEnvironmentChoose();//抽取三个环境
		void ConfirmEnvironment(int index);//确认选择，加入永久策略并进入备战
		void LoadinFromJson(const std::filesystem::path&);
	};
	extern Logic State;
}