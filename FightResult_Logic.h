#pragma once
#include<filesystem>
#include<vector>
namespace FightResult {
	class Logic {
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
	};
	extern Logic State;
}