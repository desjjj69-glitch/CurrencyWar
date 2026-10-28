#include"GameStart_Update.h"
#include"WaitingForFight_Update.h"
#include"Fight_Update.h"
#include"FightResult_Update.h"
#include"BasicGameResource.h"
#include"WaitingForFight_SaveGame.h"
#include<vector>
#include<string>
using Logger::Log;
int main() {
	BasicGameRes.init();

	bool resume = BasicGameRes.GameConfig.GameSaved == 1 &&
		WaitingForFight::SaveGame::HasSave(BasicGameRes.GamePath);
	if (resume) {
		std::vector<std::string> bossNames;
		int difficulty = 137;
		resume = WaitingForFight::SaveGame::PeekMeta(
			BasicGameRes.GamePath, bossNames, difficulty);
	}
	if (!resume) {
		// clear a stale/invalid flag so a broken save cannot loop
		if (BasicGameRes.GameConfig.GameSaved != 0) {
			BasicGameRes.GameConfig.SetGameSaved(BasicGameRes.GamePath, 0);
		}
		GameStart::Update();
	}

	while (1) {
		WaitingForFight::Update();
		Fight::Update();
		FightResult::Update();
	}
}
