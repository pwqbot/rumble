#include "flat_map.h"
#include <algorithm>
#include <boost/container/flat_map.hpp>
#include <boost/container/map.hpp>
#include <benchmark/benchmark.h>
#include <cstdint>
#include <iostream>
#include <absl/container/flat_hash_map.h>
#include <absl/container/btree_map.h>
#include <map>
#include <random>
#define NDEBUG

template <typename T>
void debug(T &&) {}

#ifdef DEBUG
template <typename T>
void debug(std::vector<T> v) {
    std::cerr << "size of vector: " << v.size() << std::endl;
    std::for_each_n(std::begin(v), std::min(size_t(10), sizeof(v)),
                    [](const auto &v) { std::cerr << v << " "; });
}
#endif

struct Update {
    uint64_t             price;
    uint64_t             volume;
    friend std::ostream &operator<<(std::ostream &out, const Update &update) {
        out << "(price:" << update.price << ",volume:" << update.volume << ")";
        return out;
    }
};

std::vector<Update> genUpdateSeq(int n) {
    std::mt19937                     engine;
    std::normal_distribution<double> distribution(10000, 200);
    std::vector<Update>              v(n);

    std::generate(std::begin(v), std::end(v), [&distribution, &engine]() {
        return Update{static_cast<uint64_t>(distribution(engine)),
                      static_cast<uint64_t>(distribution(engine))};
    });
    std::for_each(std::begin(v), std::end(v),
                  [&distribution, &engine](auto &v) {
                      auto maybeZero = [&distribution, &engine]() {
                          return int(distribution(engine)) % 2;
                      };
                      v.volume = maybeZero();
                  });

    return v;
}

template <bool need_sort, typename Map>
void ordered_lookup(const Map &mp) {
    static std::vector<int> v(100000);
    v.clear();
    for (const auto &it : mp) {
        auto x = it.first;
        v.emplace_back(x);
        /* benchmark::DoNotOptimize(x); */
    }
    if constexpr (need_sort) {
        std::sort(std::begin(v), std::end(v));
    }
    debug(v);
}

struct ok {
    
};

/* template<typename Map, typename = void> */
/* void reserve(Map& mp) { */
/* } */
/**/
/* template<> */
/* void reserve<Map, std::void_t<decltype(std::declval<Map>().reserve(1000))>>(Map& mp) { */
/*     mp.reserve(10000); */
/* } */

template <typename Map, int N, int lookup_frequency, bool need_sort>
static void BM_SomeFunction(benchmark::State &state) {
    auto seq = genUpdateSeq(N);
    Map  map;
    /* reserve(map); */
    debug(seq);
    for (auto _ : state) {
        // This code gets timed
        std::for_each(std::begin(seq), std::end(seq),
                      [&map, i = 0](const auto &update) mutable {
                          if (update.volume > 0) {
                              map[update.price] = update.volume;
                              /* std::cerr << "insert" << std::endl; */
                          } else {
                              auto it = map.find(update.price);
                              if (it != map.end()) {
                                  map.erase(it);
                              }
                              /* std::cerr << "erase" << std::endl; */
                          }
                          if ((i++ % 10000) <= lookup_frequency) {
                              ordered_lookup<need_sort>(map);
                          }
                          /* std::cerr << map.size() << std::endl; */
                      });
        std::string x = "hello";
    }
}

/* template<typename Key, typename Value> */
/* struct Ordered_unordered_map : std::unordered_map<Key, Value>{ */
/*     bool is_ordered = false; */
/* }; */

const int LOOKUP_FREQUENCY = 100;
const int N = 1000000;
// Register the function as a benchmark
BENCHMARK(BM_SomeFunction<std::map<int, int>, N, LOOKUP_FREQUENCY, false>);
BENCHMARK(
    BM_SomeFunction<std::unordered_map<int, int>, N, LOOKUP_FREQUENCY, true>);
BENCHMARK(
    BM_SomeFunction<ska::flat_hash_map<int, int>, N, LOOKUP_FREQUENCY, true>);
BENCHMARK(
    BM_SomeFunction<boost::container::flat_map<int, int>, N, LOOKUP_FREQUENCY, false>);
BENCHMARK(
    BM_SomeFunction<boost::container::map<int, int>, N, LOOKUP_FREQUENCY, false>);
BENCHMARK(
    BM_SomeFunction<absl::flat_hash_map<int, int>, N, LOOKUP_FREQUENCY, true>);
BENCHMARK(
    BM_SomeFunction<absl::btree_map<int, int>, N, LOOKUP_FREQUENCY, false>);
// Run the benchmark
BENCHMARK_MAIN();
