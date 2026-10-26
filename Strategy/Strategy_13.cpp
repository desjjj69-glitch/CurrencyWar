#include"Strategy_13.h"
#include "AutoRegister.h"
REGISTER_STRATEGY("Strategy_13", Strategy::Impl::Strategy_13);
namespace Strategy::Impl {
    void Strategy_13::OnEvent(IGameContext&, const Event&) {
        //Strategy_13 的效果在 GameStart 确认选择时处理：
        //获得 6 Money，并立刻重抽一次环境策略。
    }
    Strategy_13::Strategy_13(const nlohmann::json& p) {
        (void)p;
    }
}
