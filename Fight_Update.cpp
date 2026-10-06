#include"Fight_Update.h"
#include"Logger.h"
namespace Fight {
	void Update() {
		static Fight_Texture f;
		static bool loaded = false;
		if (!loaded) {
			loaded = true;
			f.Loadin(BasicGameRes.GamePath, BasicGameRes.GameRenderer);
			State.LoadinJson(BasicGameRes.GamePath);
		}
		State.Init();//每次进入战斗阶段单独调用一次
		Uint64 prev = SDL_GetTicks();
		while (State.AtFight != Fight_Logic::FightGlobalState::EndFight) {
			Uint64 now = SDL_GetTicks();
			Uint64 dt = now - prev;
			prev = now;
			
			State.PollEvents();
			State.DispatchEvent();
			State.Tick(dt);
			f.Draw(BasicGameRes.GameRenderer, BasicGameRes.GameFont);
			SDL_RenderPresent(BasicGameRes.GameRenderer);
			SDL_Delay(BasicGameRes.FrameTime);//渲染帧
		}
	}
}