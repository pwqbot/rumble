#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <vector>

class ThreadPool {
    using concurrency_t =
        std::invoke_result_t<decltype(std::thread::hardware_concurrency)>;

  public:
    ThreadPool(concurrency_t thread_num = 0)
        : thread_num_{get_thread_count(thread_num)} {
        create_threads(thread_num_);
    }

    ~ThreadPool() {
        {
            std::unique_lock lock(mutex_);
            stop_ = true;
        }
        condition_.notify_all();
        for (auto &worker : workers_) {
            worker.join();
        }
    }

    template <typename F, typename... Args>
    auto Push(F &&f, Args &&...args) {
        std::function<void()> task = [f        = std::forward<F>(f),
                                      ... args = std::forward<Args>(args)]() {
            f(args...);
        };
        {
            std::unique_lock lock(mutex_);
            tasks_.emplace(task);
        }
        condition_.notify_one();
        return;
    }

    template <typename F, typename... Args>
    auto PushF(F &&f, Args &&...args) {
        using R                 = std::invoke_result_t<F, Args...>;
        std::function<R()> task = [f        = std::forward<F>(f),
                                   ... args = std::forward<Args>(args)]() {
            return f(args...);
        };
        auto task_promise = std::make_shared<std::promise<R>>();
        {
            std::unique_lock lock(mutex_);
            tasks_.emplace([task_promise, task]() {
                if constexpr (std::is_void_v<R>) {
                    task();
                    (*task_promise).set_value();
                } else {
                    (*task_promise).set_value(task());
                }
            });
        }
        condition_.notify_one();
        return task_promise->get_future();
    }

  private:
    std::mutex                        mutex_;
    bool                              stop_{false};
    std::queue<std::function<void()>> tasks_;
    std::condition_variable           condition_;
    std::vector<std::thread>          workers_;
    concurrency_t                     thread_num_;

    void worker() {
        while (true) {
            std::unique_lock lock(mutex_);
            condition_.wait(lock, [this] { return !tasks_.empty() || stop_; });
            if (stop_ && tasks_.empty()) {
                return;
            }
            auto task = std::move(tasks_.front());
            tasks_.pop();
            lock.unlock();
            task();
        }
    }

    void create_threads(concurrency_t thread_num) {
        for (int i = 0; i < thread_num; i++) {
            workers_.emplace_back([this] { worker(); });
        }
    }

    auto get_thread_count(const concurrency_t thread_num) -> concurrency_t {
        if (thread_num > 0) {
            return thread_num;
        } else {
            return std::thread::hardware_concurrency();
        }
    }
};

int main() {
    ThreadPool pool{};
    int        i{};
    int        j      = 0;
    auto       future = pool.PushF([&i, &j] {
        for (int k = 0; k < 100000; k++) {
            j += i;
            /* std::cerr << j << std::endl; */
        }
    });
    pool.Push([&i] { std::cerr << i++ << std::endl; });
    pool.Push([&i] { std::cerr << i++ << std::endl; });
    future.get();
    pool.Push([&i] { std::cerr << i++ << std::endl; });
    pool.Push([&i] { std::cerr << i++ << std::endl; });
    pool.PushF([&i] { std::cerr << i++ << std::endl; });
    return 0;
}
