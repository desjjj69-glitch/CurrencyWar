#include"GameStart_Update.h"
#include"WaitingForFight_Update.h"
#include"Fight_Update.h"
#include"FightResult_Update.h"
using Logger::Log;
int main() {
	GameStart::Update();
	while (1) {
		WaitingForFight::Update();
		Fight::Update();
		FightResult::Update();
	}
	
	
	
	
}