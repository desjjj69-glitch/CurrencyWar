#include"Strategy_9.h"
#include "AutoRegister.h"
#include "../WaitingForFight_InteractLogic.h"
REGISTER_STRATEGY("Strategy_9", Strategy::Impl::Strategy_9);
namespace Strategy::Impl {
    void Strategy_9::OnEvent(IGameContext& ctx, const Event& e) {
        if (granted_ || e.type != EventType::PrepStageBegin) return;
        if (ctx.Kind() != StageKind::WaitingForFight)return;
        auto* logic = dynamic_cast<WaitingForFight::Logic*>(&ctx);
        if (!logic)return;
        logic->AddMineralCharacter(4);
        granted_ = true;
    }
    Strategy_9::Strategy_9(const nlohmann::json& p) {
        (void)p;
    }
}
