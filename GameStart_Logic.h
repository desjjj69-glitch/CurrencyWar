#pragma once
#include<SDL3/SDL.h>
#include<filesystem>
#include<string>
namespace GameStart {
	class Logic {
	public:
		std::vector<std::string> Boss_name;
		void Init();

		void EventLogic();
		enum class State {
			Continue,
			Waiting
		};
		State state = State::Waiting;
	private:
		void LoadinFromJson(const std::filesystem::path&);
	};
	extern Logic State;
}