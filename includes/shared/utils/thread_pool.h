#pragma once

#include <mutex>
#include <thread>
#include <queue>
#include <vector>
#include <functional>
#include <future>
#include <condition_variable>
#include <stdexcept>

class ThreadPool 
{
public:
    explicit ThreadPool(size_t numThreads);
    ~ThreadPool();

    /// @brief Enqueue a job, returns std::future for the result.
    template<typename F>
    auto Enqueue(F&& job) -> std::future<std::invoke_result_t<F>>;

    /// @brief Wait for all enqueued jobs to complete.
    void WaitAll();

private:
    std::vector<std::thread>             m_workers;
    std::queue<std::function<void()>>    m_jobs;
    std::mutex                           m_mutex;
    std::condition_variable              m_condition;
    std::condition_variable              m_doneCondition;
    int                                  m_activeWorkers = 0;
    bool                                 m_stop = false;
};

template<typename F>
auto ThreadPool::Enqueue(F&& job) -> std::future<std::invoke_result_t<F>>
{
    using ReturnType = std::invoke_result_t<F>;

    auto task = std::make_shared<std::packaged_task<ReturnType()>>(
        std::forward<F>(job)
    );

    std::future<ReturnType> result = task->get_future();

    {
        std::lock_guard<std::mutex> lock(this->m_mutex);

        if (this->m_stop)
        {
            throw std::runtime_error("Cannot enqueue on a stopped ThreadPool");
        }

        this->m_jobs.emplace([task]() { (*task)(); });
    }

    this->m_condition.notify_one();
    return result;
}
