#include"Strategy_TestStrategy3.h"
#include "AutoRegister.h"
REGISTER_STRATEGY("TestStrategy3", Strategy::Impl::TestStrategy3);
namespace Strategy::Impl {
    void TestStrategy3::OnEvent(IGameContext& ctx, const Event& e) {
        if (e.type != EventType::PrepStageBegin) return;
        if (ctx.Kind() != StageKind::WaitingForFight)return;
        //TODO 补完策略内容
    }
    TestStrategy3::TestStrategy3(const nlohmann::json& p) {
        (void)p;
    }
}
