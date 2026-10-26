#include"Strategy_6.h"
#include "AutoRegister.h"
#include "../WaitingForFight_InteractLogic.h"
REGISTER_STRATEGY("Strategy_6", Strategy::Impl::Strategy_6);
namespace Strategy::Impl {
    void Strategy_6::OnEvent(IGameContext& ctx, const Event& e) {
        if (e.type != EventType::ShopRefreshed) return;
        ++refresh_count_;
        if (granted_ || refresh_count_ < 20) return;
        if (ctx.Kind() != StageKind::WaitingForFight)return;
        auto* logic = dynamic_cast<WaitingForFight::Logic*>(&ctx);
        if (!logic)return;
        logic->AddMoney(30);
        granted_ = true;
    }
    Strategy_6::Strategy_6(const nlohmann::json& p) {
        (void)p;
    }
}
