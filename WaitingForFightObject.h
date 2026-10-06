#pragma once
#include"CharacterObject.h"
#include<vector>
#include<memory>
class WaitingForFightObject {
public:
	std::vector<std::unique_ptr<BasicObject> >objects;
	
	enum Mode {
		Idle,
		Pressed,
		Dragging
	};
	Mode mode = Idle;
	
	std::unique_ptr<BasicObject> holding;
	size_t OriginIndex = 0;
	SDL_FRect OriginPlace = {};
	SDL_FRect Offset = {};
	
	Uint64 PressStartTime = 0;
	static constexpr Uint64 LONG_PRESS_MS = 350;
	void MouseDown(const SDL_MouseButtonEvent& e, Uint64 nowTime);
	void MouseUp(const SDL_MouseButtonEvent& e);
	void MouseMove(const SDL_MouseButtonEvent& e);
};