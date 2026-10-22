#pragma once
#include<filesystem>
#include<vector>
#include<string>
#include<unordered_map>
#include"SDL_Functions.h"
namespace FightResult {
	class Logic {
	private:
		SDL_FRect GetUIRect(const std::string&,
			std::size_t);
	public:
		bool AtFightResult = false;
		void Handle();
		void EventLogic();
		void LoadinJson(const std::filesystem::path&);
		std::vector<std::vector<int>> BasicMoneyGain;
		std::vector<int> ContinueWinMoneyGain;

		std::vector<std::vector<int>>ExpGain;
		std::vector<std::vector<int>> BloodGain;
	private:
		void HandleMoney(int&,int,int,size_t,size_t);
		void HandleExp(int&, size_t, size_t);
		void HandleBlood(int&, size_t, size_t);

		std::unordered_map<std::string, std::vector<SDL_FRect>> ui_place;
	};
	extern Logic State;
}