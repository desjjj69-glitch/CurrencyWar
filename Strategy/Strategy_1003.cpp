#include"Strategy_1003.h"
#include "AutoRegister.h"
REGISTER_STRATEGY("Strategy_1003", Strategy::Impl::Strategy_1003);
namespace Strategy::Impl {
    void Strategy_1003::OnEvent(IGameContext&, const Event&) {
        //阶段策略测试占位
    }
    Strategy_1003::Strategy_1003(const nlohmann::json& p) {
        (void)p;
    }
}
