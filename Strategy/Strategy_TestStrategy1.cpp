#include"Strategy_TestStrategy1.h"
#include "AutoRegister.h"
REGISTER_STRATEGY("TestStrategy1", Strategy::Impl::TestStrategy1);
namespace Strategy::Impl {
    void TestStrategy1::OnEvent(IGameContext& ctx, const Event& e) {
        if (e.type != EventType::PrepStageBegin) return;    // 只在进入备战节点时触发
        if (ctx.Kind() != StageKind::WaitingForFight)return;
        //TODO 补完策略内容
    }
    TestStrategy1::TestStrategy1(const nlohmann::json& p) {

    }
}