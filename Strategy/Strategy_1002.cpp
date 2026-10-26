#include"Strategy_1002.h"
#include "AutoRegister.h"
REGISTER_STRATEGY("Strategy_1002", Strategy::Impl::Strategy_1002);
namespace Strategy::Impl {
    void Strategy_1002::OnEvent(IGameContext&, const Event&) {
        //阶段策略测试占位
    }
    Strategy_1002::Strategy_1002(const nlohmann::json& p) {
        (void)p;
    }
}
