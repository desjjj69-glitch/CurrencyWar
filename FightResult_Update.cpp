#include"FightResult_Update.h"
#include"FightResult_Logic.h"
#include"FightResult_Texture.h"
#include"Logger.h"
#include"BasicGameResource.h"
#include"SDL_Functions.h"
#include<iostream>
namespace FightResult {
	void Update() {
		static FightResult::FightResult_Texture f;
		static bool loaded = false;
		if (!loaded) {
			loaded = true;
			f.Loadin(BasicGameRes.GamePath, BasicGameRes.GameRenderer);
			State.LoadinJson(BasicGameRes.GamePath);
		}
		State.AtFightResult = true;
		State.Handle();//这个阶段的游戏资源逻辑一开始就结算完了
		while (1) {
			State.EventLogic();//这里只是处理鼠标逻辑，比如查看等基本逻辑
			if (State.AtFightResult == false) {
				break;
			}
			f.Draw(BasicGameRes.GameRenderer, BasicGameRes.GameFont);
			SDL_RenderPresent(BasicGameRes.GameRenderer);
			SDL_Delay(BasicGameRes.FrameTime);
		}
	}
}