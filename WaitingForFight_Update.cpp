#include"WaitingForFight_Update.h"
#include"WaitingForFight_SaveGame.h"
#include"Logger.h"
using Logger::Log;
namespace WaitingForFight{
	void Update() {
		static WaitingForFight::WaitingForFightTexture w;
		static bool loaded = false;
		static bool firstEntry = true;
		static bool resumedEntry = false;

		if (!loaded) {
			loaded = true;
			WaitingForFight::State.LoadinJson(BasicGameRes.GamePath);
			w.Loadin(BasicGameRes.GamePath, BasicGameRes.GameRenderer);

			if (BasicGameRes.GameConfig.GameSaved == 1 &&
				SaveGame::HasSave(BasicGameRes.GamePath)) {
				if (SaveGame::Load(WaitingForFight::State, BasicGameRes.GamePath)) {
					WaitingForFight::State.AtWaitingForFight = true;
					resumedEntry = true;
					BasicGameRes.GameConfig.SetGameSaved(BasicGameRes.GamePath, 0);
					Log("Resume run from save", __func__, Logcode::OK);
				}
				else {
					Log("Resume failed, starting a new run", __func__, Logcode::ERROR);
					BasicGameRes.GameConfig.SetGameSaved(BasicGameRes.GamePath, 0);
				}
			}
		}

		// Resume already restored the preparation state; every other entry
		// advances to the next node as before.
		if (!(firstEntry && resumedEntry)) {
			WaitingForFight::State.Init();
		}
		firstEntry = false;

		SDL_Event event;
		while (1) {
			w.Draw(BasicGameRes.GameRenderer, BasicGameRes.GameFont);
			SDL_RenderPresent(BasicGameRes.GameRenderer);
			WaitingForFight::State.EventLogic(event);
			if (!State.AtWaitingForFight) {
				return;
			}
			SDL_Delay(BasicGameRes.FrameTime);
		}
	}
}
