//
// Created by steve on 3/9/2022.
//

#pragma once

#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <vector>

namespace tp {

template <class T>
class ThreadPool {
  public:
    explicit ThreadPool(size_t, std::vector<T> &&);
    template <class F, class... Args>
    auto enqueue(F &&f, Args &&...args)
        -> std::future<std::invoke_result_t<F, T &, Args...>>;
    ~ThreadPool();
    auto &get_stacks();

    enum class Status { STOP, PAUSE, RUNNING };

    void pause() {
        std::unique_lock<std::mutex> lock(queue_mutex);
        // SPDLOG_INFO("threadpool paused");
        status = Status::PAUSE;
        condition.wait_for(lock, std::chrono::nanoseconds(1),
                           [this] { return n_thread_wait == workers.size(); });
    }

    void run() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            // SPDLOG_INFO("threadpool resumed");
            status = Status::RUNNING;
        }
        condition.notify_all();
    }

  private:
    std::vector<T> stacks;
    // need to keep track of threads, so we can join them
    std::vector<std::thread> workers;
    // the task queue
    std::queue<std::function<void(T &)>> tasks;
    Status                               status;
    // synchronization
    std::mutex              queue_mutex;
    std::condition_variable condition;
    std::size_t             n_thread_wait;
};
template <class T>
inline auto &ThreadPool<T>::get_stacks() {
    return this->stacks;
}
// the constructor just launches some amount of workers
template <class T>
inline ThreadPool<T>::ThreadPool(size_t threads, std::vector<T> &&v)
    : stacks(v), status(Status::RUNNING), n_thread_wait(0) {
    for (size_t i = 0; i < threads; ++i)
        workers.emplace_back([this, i] {
            for (;;) {
                std::unique_lock<std::mutex> lock(queue_mutex);
                n_thread_wait += 1;
                condition.wait(lock, [this] {
                    return status == Status::STOP ||
                           (status != Status::PAUSE && !tasks.empty());
                });
                n_thread_wait -= 1;
                if (status == Status::STOP && tasks.empty()) {
                    return;
                } else {
                    std::function<void(T &)> task = std::move(tasks.front());
                    tasks.pop();
                    lock.unlock();
                    task(stacks[i]);
                }
            }
        });
}

// add new work item to the pool
template <class T>
template <class F, class... Args>
auto ThreadPool<T>::enqueue(F &&f, Args &&...args)
    -> std::future<std::invoke_result_t<F, T &, Args...>> {
    using return_type = std::invoke_result_t<F, T &, Args...>;
    auto bind_f       = [... args= std::forward<Args>(args),
                   f = std::forward<std::decay_t<F>>(f)](T &t) mutable {
        return f(t, args...);
    };
    auto task = std::make_shared<std::packaged_task<return_type(T &)>>(
        std::move(bind_f));

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);

        // don't allow enqueueing after stopping the pool
        if (status == Status::STOP) {
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }

        tasks.emplace([task](T &s) { (*task)(s); });
    }
    condition.notify_one();
    return res;
}

// the destructor joins all threads
template <class T>
inline ThreadPool<T>::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        status = Status::STOP;
    }
    condition.notify_all();
    for (std::thread &worker : workers) {
        worker.join();
    }
}
} // namespace tp
