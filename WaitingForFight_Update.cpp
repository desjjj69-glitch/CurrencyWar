#include"WaitingForFight_Update.h"
#include"Logger.h"
namespace WaitingForFight{
	void Update() {
		static WaitingForFight::WaitingForFightTexture w;
		static bool loaded = false;
		
		if (!loaded) {
			loaded = true;
			WaitingForFight::State.LoadinJson(BasicGameRes.GamePath);
			w.Loadin(BasicGameRes.GamePath, BasicGameRes.GameRenderer);
			
		}
		WaitingForFight::State.Init();//加载文件之后，每个阶段开始时都有一个初始化过程
		SDL_Event event;//event无所谓，反正都要被Poll的
		
		while (1) {
			//Log("StartDraw", __func__);
			w.Draw(BasicGameRes.GameRenderer, BasicGameRes.GameFont);
			SDL_RenderPresent(BasicGameRes.GameRenderer);
			//Log("Drawed", __func__);
			WaitingForFight::State.EventLogic(event);
			if (!State.AtWaitingForFight) {
				return;
			}
			SDL_Delay(BasicGameRes.FrameTime);
		}
	}
}