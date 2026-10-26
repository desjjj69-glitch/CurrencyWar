// Strategy/StrategyRegistry.h
#pragma once
#include <string>
#include <functional>
#include <memory>
#include <unordered_map>
#include"../json.hpp"
#include "StrategyBus.h"

namespace Strategy {

    // 策略统一构造签名：接收本策略的 json 参数块（没有参数就传空 json）
    using StrategyFactory = std::function<std::unique_ptr<IStrategy>(const nlohmann::json& params)>;

    class Registry {
    public:
        static Registry& Instance();

        // ① 工厂注册：由各策略的 .cpp 里的静态注册宏调用（见下）
        void Register(const std::string& id, StrategyFactory f);

        // ② 加载参数表：读 strategy_pool.json，把每个 id 的 params 块缓存起来
        void LoadPool(const std::filesystem::path& gp);

        // ③ 实例化：id + 缓存的 params → 策略对象
        //    返回 nullptr 表示 id 未注册或参数加载失败
        std::unique_ptr<IStrategy> Make(const std::string& id) const;

        // 用于"给出三选一"的界面：随机抽 n 个已注册的 id
        // exclude 中的 id 不会被抽出（例如已经获得过的策略）
        // kind 非空时，只抽取 params.Kind == kind 的策略
        std::vector<std::string> PickRandom(
            int n,
            const std::vector<std::string>& exclude = {},
            const std::string& kind = "") const;

    private:
        std::unordered_map<std::string, StrategyFactory> factories_;
        std::unordered_map<std::string, nlohmann::json>  params_;     // id -> 参数块
        std::vector<std::string>                          allIds_;     // 用于随机抽取
    };

} // namespace Strategy