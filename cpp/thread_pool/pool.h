#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <spdlog/spdlog.h>
#include <thread>
#include <type_traits>
#include <vector>

class ThreadPool {
    using concurrency_t =
        std::invoke_result_t<decltype(std::thread::hardware_concurrency)>;
    enum class Status { STOP, PAUSE, RUNNING };

  public:
    explicit ThreadPool(concurrency_t thread_num = 0)
        : thread_num_{get_thread_count(thread_num)} {
        create_threads(thread_num_);
    }

    ~ThreadPool() {
        {
            std::unique_lock lock(mutex_);
            status_ = Status::STOP;
        }
        work_condition_.notify_all();
        for (auto &worker : workers_) {
            worker.join();
        }
    }

    ThreadPool(const ThreadPool &)                   = delete;
    auto operator=(const ThreadPool &) -> ThreadPool = delete;
    ThreadPool(ThreadPool &&)                        = delete;
    auto operator=(ThreadPool &&) -> ThreadPool      = delete;

    void Pause(bool sync_wait = true) {
        spdlog::info("Try to pause");
        hp_flag_ = true;
        std::unique_lock lock(mutex_);
        hp_flag_ = false;
        status_  = Status::PAUSE;
        spdlog::info("Wait for all workers pause, wait_num {}, workers {}",
                     wait_num_, workers_.size());
        if (sync_wait) {
            pause_condition_.wait(
                lock, [this]() { return wait_num_ == workers_.size(); });
        }
    }

    void Run() {
        {
            std::unique_lock lock(mutex_);
            status_ = Status::RUNNING;
        }
        work_condition_.notify_all();
    }

    // TODO(ding.wang): fix this
    // void WaitAllDone() {
    //     std::unique_lock lock(mutex_);
    //     if (status_ == Status::PAUSE) {
    //         return;
    //     }
    //     spdlog::info("Wait for all tasks done, {} {}", working_num_,
    //                  tasks_.size());
    //     done_condition_.wait(lock, [this]() {
    //         // spdlog::info("{} {}", working_num_, tasks_.size());
    //         return working_num_ == 0 && tasks_.empty();
    //     });
    // }

    template <typename F, typename... Args>
    auto Push(F &&f, Args &&...args) {
        auto task = [f        = std::forward<F>(f),
                     ... args = std::forward<Args>(args)]() mutable {
            f(std::move(args)...);
        };
        {
            std::unique_lock lock(mutex_);
            work_condition_.wait(lock, [this]() { return !hp_flag_; });
            tasks_.emplace([task = std::move(task)]() mutable { task(); });
        }
        work_condition_.notify_one();
        return;
    }

    template <typename F, typename... Args>
    auto PushF(F &&f, Args &&...args) {
        using R            = std::invoke_result_t<F, Args...>;
        auto bind_function = [f        = std::forward<std::decay_t<F>>(f),
                              ... args = std::forward<Args>(args)]() mutable {
            return f(std::move(args)...);
        };
        auto task =
            std::make_shared<std::packaged_task<R()>>(std::move(bind_function));
        auto task_promise = task->get_future();
        {
            std::unique_lock lock(mutex_);
            work_condition_.wait(lock, [this]() { return !hp_flag_; });
            tasks_.emplace([task]() { (*task)(); });
        }
        work_condition_.notify_one();
        return task_promise;
    }

  private:
    std::mutex                        mutex_;
    std::queue<std::function<void()>> tasks_;
    std::condition_variable           work_condition_;
    std::condition_variable           done_condition_;
    std::condition_variable           pause_condition_;
    std::vector<std::thread>          workers_;
    concurrency_t                     thread_num_{};
    concurrency_t                     wait_num_{0};
    std::atomic<concurrency_t>        working_num_{0};
    Status                            status_{Status::RUNNING};
    std::atomic<bool>                 hp_flag_{false};

    void tryNotifyPause() {
        if (status_ == Status::PAUSE && wait_num_ == workers_.size()) {
            pause_condition_.notify_one();
        }
    }

    void worker() {
        while (true) {
            std::unique_lock lock(mutex_);
            wait_num_++;
            tryNotifyPause();
            work_condition_.wait(lock, [this] {
                return !hp_flag_ &&
                       ((status_ != Status::PAUSE && !tasks_.empty()) ||
                        status_ == Status::STOP);
            });
            wait_num_--;
            if (status_ == Status::STOP && tasks_.empty()) {
                return;
            }
            working_num_.fetch_add(1);
            auto task = std::move(tasks_.front());
            tasks_.pop();

            lock.unlock();

            task();
            working_num_.fetch_sub(1);
            if (working_num_ == 0) {
                done_condition_.notify_all();
            }
        }
    }

    void create_threads(concurrency_t thread_num) {
        for (int i = 0; i < thread_num; i++) {
            workers_.emplace_back([this] { worker(); });
        }
    }

    static auto get_thread_count(const concurrency_t thread_num)
        -> concurrency_t {
        if (thread_num > 0) {
            return thread_num;
        }
        return std::thread::hardware_concurrency();
    }
};
