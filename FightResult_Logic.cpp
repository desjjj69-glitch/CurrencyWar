#include"FightResult_Logic.h"
#include"StageData.h"
#include"SDL_Functions.h"
#include"BasicGameResource.h"
namespace FightResult {
	Logic State;
	void Logic::Handle() {
		++(*StageData::g_WaitingForFight_publicData.stage_index);//阶段+1，如果没有blood归零的话
	}
	void Logic::EventLogic() {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			SDL_ConvertEventToRenderCoordinates(BasicGameRes.GameRenderer, &event);
			switch (event.type) {
			case SDL_EVENT_KEY_DOWN:
				switch (event.key.scancode) {
				case SDL_SCANCODE_ESCAPE:
					AtFightResult = false;
					break;
				}
				break;
			}
		}
	}
}