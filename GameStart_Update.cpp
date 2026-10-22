#include"GameStart_Update.h"
#include"GameStart_Logic.h"
#include"GameStart_Texture.h"
#include"BasicGameResource.h"
namespace GameStart {
	void Update() {
		BasicGameRes.init();
		Logic State;
		Texture tex;
		State.Init();
		tex.Loadin(BasicGameRes.GamePath, BasicGameRes.GameRenderer);
		while (State.state!=Logic::State::Continue) {
			State.EventLogic();
			
			tex.Draw(BasicGameRes.GameRenderer, BasicGameRes.GameFont);
			SDL_Delay(BasicGameRes.FrameTime);
		}
	}
}