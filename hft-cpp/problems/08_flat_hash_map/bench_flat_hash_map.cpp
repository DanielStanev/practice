// ./run_tests.sh bench
#include <cstdint>
#include <random>
#include <unordered_map>
#include <vector>

#include "bench.hpp"
#include "flat_hash_map.hpp"

int main() {
    constexpr int kKeys = 100'000;
    constexpr int kLookups = 2'000'000;
    std::mt19937_64 rng(5);
    std::vector<std::uint64_t> keys(kKeys);
    for (auto& k : keys) k = rng();
    std::vector<std::uint64_t> queries(kLookups);
    for (auto& q : queries) q = (rng() % 2) ? keys[rng() % kKeys] : rng();  // ~50% hits

    std::unordered_map<std::uint64_t, std::uint64_t> um;
    hft::FlatHashMap<std::uint64_t, std::uint64_t> fm;
    for (auto k : keys) {
        um.emplace(k, k);
        fm.insert(k, k);
    }

    bench::run("std::unordered_map find (50% hit)", kLookups, [&] {
        std::uint64_t sum = 0;
        for (auto q : queries) {
            auto it = um.find(q);
            if (it != um.end()) sum += it->second;
        }
        bench::do_not_optimize(sum);
    });
    bench::run("FlatHashMap find (50% hit)", kLookups, [&] {
        std::uint64_t sum = 0;
        for (auto q : queries) {
            if (auto* v = fm.find(q)) sum += *v;
        }
        bench::do_not_optimize(sum);
    });
    bench::run("std::unordered_map insert+erase churn", kKeys, [&] {
        for (int i = 0; i < kKeys; ++i) {
            um.erase(keys[i]);
            um.emplace(keys[i], i);
        }
    });
    bench::run("FlatHashMap insert+erase churn", kKeys, [&] {
        for (int i = 0; i < kKeys; ++i) {
            fm.erase(keys[i]);
            fm.insert(keys[i], i);
        }
    });
}
