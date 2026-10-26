// Strategy/StrategyRegistry.cpp
#include "StrategyRegistry.h"
#include "../Logger.h"
#include"../GetRandomNum.h"
#include <fstream>
#include<random>
namespace Strategy {

    Registry& Registry::Instance() {
        static Registry r;
        return r;
    }

    void Registry::Register(const std::string& id, StrategyFactory f) {
        factories_[id] = std::move(f);
    }

    void Registry::LoadPool(const std::filesystem::path& gp) {
        std::ifstream in(gp / "config" / "strategy_pool.json");
        if (!in.is_open()) {
            Logger::Log("strategy_pool.json open failed", __func__, Logcode::OPENFAILED);
            return;
        }
        nlohmann::json js;
        in >> js;

        // 约定格式：{ "strategies": [ { "id": "...", "params": {...} }, ... ] }
        for (auto& item : js.at("strategies")) {
            const std::string id = item.at("id").get<std::string>();
            params_[id] = item.value("params", nlohmann::json::object());

            // 只把"已注册工厂"的 id 纳入可抽取池，避免抽到没实现的
            if (factories_.count(id)) allIds_.push_back(id);
        }
    }

    std::unique_ptr<IStrategy> Registry::Make(const std::string& id) const {
        auto fit = factories_.find(id);
        if (fit == factories_.end()) {
            Logger::Log("strategy not registered: " + id, __func__, Logcode::NOTFOUND);
            return nullptr;
        }
        auto pit = params_.find(id);
        const nlohmann::json params = (pit != params_.end()) ? pit->second : nlohmann::json::object();
        return fit->second(params);
    }

    std::vector<std::string> Registry::PickRandom(int n, const std::vector<std::string>& exclude) const {
        std::vector<std::string> pool;
        pool.reserve(allIds_.size());
        for (const auto& id : allIds_) {
            if (std::find(exclude.begin(), exclude.end(), id) == exclude.end()) {
                pool.push_back(id);
            }
        }

        const int total = static_cast<int>(pool.size());
        if (n <= 0 || total == 0) return {};

        const int take = std::min(n, total);   // n > 总数时取总数，不越界

        // 部分 Fisher-Yates：只打乱前 take 个位置
        // 第 i 轮从 [i, total-1] 随机挑一个换到 i，i 之前的位置已是结果
        for (int i = 0; i < take; ++i) {
            int j = GetRandomNum(i, total - 1);
            std::swap(pool[i], pool[j]);
        }
        pool.resize(take);
        return pool;
    }

} // namespace Strategy