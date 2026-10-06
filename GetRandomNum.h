#pragma once
#include<random>
#include<vector>
#include<unordered_set>
inline std::mt19937& get_random_engine() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    return gen;
}
inline int GetRandomNum(int left, int right) {
    std::uniform_int_distribution<int> dist(left, right);
    return dist(get_random_engine());
}
inline long long GetRandomNum(long long left, long long right) {
    std::uniform_int_distribution<long long> dist(left, right);
    return dist(get_random_engine());
}
inline std::vector<int> GetRandomDistinct(int n, int count) {
    if (n <= 0 || count <= 0) return {};
    if (count > n) count = n;

    std::vector<int> res;
    res.reserve(count);
    if (count == 0) return res;

    std::unordered_set<int> chosen;
    chosen.reserve(count * 2);

    for (int j = n - count; j < n; ++j) {
        int t = GetRandomNum(0, j);           // [0, j]
        if (chosen.count(t)) {
            chosen.insert(j);                 // t 已被选过 → 换成 j
            res.push_back(j);
        }
        else {
            chosen.insert(t);
            res.push_back(t);
        }
    }
    return res;
}