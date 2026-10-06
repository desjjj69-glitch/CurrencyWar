#include<SDL3/SDL.h>
#include"Logger.h"
#include"BasicGameResource.h"
#include"WaitingForFight_Update.h"
#include"Fight_Update.h"
#include"FightResult_Update.h"
using Logger::Log;
int main() {
	BasicGameRes.init();
	while (1) {
		WaitingForFight::Update();
		Fight::Update();
		FightResult::Update();
	}
	
	
	
	
}