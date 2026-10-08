#include"Fight_Update.h"
#include"Logger.h"
namespace Fight {
	void Update() {
		Fight_Texture f;
			
			
		State.LoadinJson(BasicGameRes.GamePath);
		State.Init();//每次进入战斗阶段单独调用一次
		f.Loadin(BasicGameRes.GamePath, BasicGameRes.GameRenderer);//现在f是局部变量，每次都要重新加载
		Uint64 prev = SDL_GetTicks();
		while (State.AtFight != Fight_Logic::FightGlobalState::EndFight) {
			Uint64 now = SDL_GetTicks();
			Uint64 dt = now - prev;
			prev = now;
			switch (State.AtFight) {
			case Fight_Logic::FightGlobalState::AtFight:
				State.PollEvents();
				State.DispatchEvent();
				State.Tick(dt);
				break;
			case Fight_Logic::FightGlobalState::Pause:
				State.Pause();
				break;
			}
			
			f.Draw(BasicGameRes.GameRenderer, BasicGameRes.GameFont);
			SDL_RenderPresent(BasicGameRes.GameRenderer);
			SDL_Delay(BasicGameRes.FrameTime);//渲染帧
		}
	}
}