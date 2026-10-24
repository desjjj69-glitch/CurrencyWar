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
		void Init();

		void EventLogic();
		enum class State {
			Continue,
			Waiting
		};
		State state = State::Waiting;
	private:
		static bool PlaceInside(float x, float y, const SDL_FRect& rect) {
			return x >= rect.x && x <= rect.x + rect.w && y >= rect.y && y <= rect.y + rect.h;
		}
		void LoadinFromJson(const std::filesystem::path&);
	};
	extern Logic State;
}