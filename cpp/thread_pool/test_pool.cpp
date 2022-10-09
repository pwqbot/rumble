#include "d_pool.h"
#include "pool.h"
#include <gtest/gtest.h>
#include <spdlog/spdlog.h>
#include <thread>
#include <unistd.h>

TEST(Pool, Push) {
    std::atomic<int> i{};
    {
        ThreadPool pool{};
        pool.Push([](std::atomic<int> &i) { i++; }, std::ref(i));
        pool.Push([&i] { i++; });
        pool.Push([&i] { i++; });
    }
    EXPECT_EQ(i, 3);
}

TEST(Pool, PushF) {
    ThreadPool                     pool{};
    std::vector<std::future<void>> fus;
    std::atomic<int>               i{};
    fus.emplace_back(pool.PushF([](std::atomic<int> &i) { i++; }, std::ref(i)));
    fus.emplace_back(pool.PushF([&i] { i++; }));
    fus.emplace_back(pool.PushF([&i] { i++; }));
    for (auto &fu : fus) {
        fu.get();
    }
    EXPECT_EQ(i, 3);
}

TEST(Pool, Pause) {
    ThreadPool pool{};

    std::atomic<int> i{};
    {
        std::vector<std::future<void>> fus;
        fus.emplace_back(
            pool.PushF([](std::atomic<int> &i) { i++; }, std::ref(i)));
        fus.emplace_back(pool.PushF([&i] { i++; }));
        fus.emplace_back(pool.PushF([&i] { i++; }));
        for (auto &fu : fus) {
            fu.get();
        }
        EXPECT_EQ(i, 3);
    }

    pool.Pause(true);
    {
        std::vector<std::future<void>> fus;
        fus.emplace_back(
            pool.PushF([](std::atomic<int> &i) { i++; }, std::ref(i)));
        fus.emplace_back(pool.PushF([&i] { i++; }));
        fus.emplace_back(pool.PushF([&i] { i++; }));
        std::this_thread::sleep_for(std::chrono::milliseconds{1});
        EXPECT_EQ(i, 3);
        pool.Run();

        for (auto &fu : fus) {
            fu.get();
        }
        EXPECT_EQ(i, 6);
    }
}

TEST(Pool, Move) {
    ThreadPool pool{};
    struct A {
        explicit A(int xx) : x{xx} {}
        [[nodiscard]] auto wang() const -> int { return x + 1; }
        int                x;
    };
    auto a = std::make_unique<A>(1);
    auto f = pool.PushF([](std::unique_ptr<A> a) -> int { return a->wang(); },
                        std::move(a));
    EXPECT_EQ(f.get(), 2);
}
