#pragma once
namespace FightResult {
	class Logic {
	public:
		bool AtFightResult = false;
		void Handle();
		void EventLogic();
	};
	extern Logic State;
}