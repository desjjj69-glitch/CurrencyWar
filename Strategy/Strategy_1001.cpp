#include"Strategy_1001.h"
#include "AutoRegister.h"
REGISTER_STRATEGY("Strategy_1001", Strategy::Impl::Strategy_1001);
namespace Strategy::Impl {
    void Strategy_1001::OnEvent(IGameContext&, const Event&) {
        //阶段策略测试占位
    }
    Strategy_1001::Strategy_1001(const nlohmann::json& p) {
        (void)p;
    }
}
