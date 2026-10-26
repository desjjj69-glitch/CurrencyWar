#include"Strategy_TestStrategy2.h"
#include "AutoRegister.h"
REGISTER_STRATEGY("TestStrategy2", Strategy::Impl::TestStrategy2);
namespace Strategy::Impl {
    void TestStrategy2::OnEvent(IGameContext& ctx, const Event& e) {
        if (e.type != EventType::PrepStageBegin) return;
        if (ctx.Kind() != StageKind::WaitingForFight)return;
        //TODO 补完策略内容
    }
    TestStrategy2::TestStrategy2(const nlohmann::json& p) {
        (void)p;
    }
}
