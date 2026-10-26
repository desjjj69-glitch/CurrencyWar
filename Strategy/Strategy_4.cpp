#include"Strategy_4.h"
#include "AutoRegister.h"
#include "../WaitingForFight_InteractLogic.h"
REGISTER_STRATEGY("Strategy_4", Strategy::Impl::Strategy_4);
namespace Strategy::Impl {
    void Strategy_4::OnEvent(IGameContext& ctx, const Event& e) {
        if (e.type != EventType::WinStreakChanged) return;
        if (granted_ || e.amount != 5) return;
        if (ctx.Kind() != StageKind::WaitingForFight)return;
        auto* logic = dynamic_cast<WaitingForFight::Logic*>(&ctx);
        if (!logic)return;
        logic->AddEquipmentByName("Wealth Keeper's Dream");
        granted_ = true;
    }
    Strategy_4::Strategy_4(const nlohmann::json& p) {
        (void)p;
    }
}
