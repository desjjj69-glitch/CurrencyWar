#include"Strategy_TestStrategy1.h"
#include "AutoRegister.h"
#include "../WaitingForFight_InteractLogic.h"
REGISTER_STRATEGY("Strategy_3", Strategy::Impl::Strategy_3);
namespace Strategy::Impl {
    void Strategy_3::OnEvent(IGameContext& ctx, const Event& e) {
        if (e.type != EventType::DimensionChanged) return;
        if (ctx.Kind() != StageKind::WaitingForFight)return;
        auto* logic = dynamic_cast<WaitingForFight::Logic*>(&ctx);
        if (!logic)return;

        int money = 0;
        switch (e.amount) {
        case 0: money = 6; break;
        case 1: money = 8; break;
        case 2: money = 12; break;
        default: return;
        }
        logic->AddMineralMoney(money);
    }
    Strategy_3::Strategy_3(const nlohmann::json& p) {
        (void)p;
    }
}
