#include "includes/shared/utils/thread_pool.h"

ThreadPool::ThreadPool(size_t numThreads)
{
    if (numThreads == 0)
    {
        numThreads = 1;
    }

    this->m_workers.reserve(numThreads);

    for (size_t i = 0; i < numThreads; i++)
    {
        this->m_workers.emplace_back([this]()
        {
            while (true)
            {
                std::function<void()> job;

                {
                    std::unique_lock<std::mutex> lock(this->m_mutex);

                    this->m_condition.wait(lock, [this]()
                    {
                        return this->m_stop || !this->m_jobs.empty();
                    });

                    if (this->m_stop && this->m_jobs.empty())
                    {
                        return;
                    }

                    job = std::move(this->m_jobs.front());
                    this->m_jobs.pop();
                    this->m_activeWorkers++;
                }

                try
                {
                    job();
                }
                catch (...)
                {
              
                }

                {
                    std::lock_guard<std::mutex> lock(this->m_mutex);
                    this->m_activeWorkers--;

                    if (this->m_jobs.empty() && this->m_activeWorkers == 0)
                    {
                        this->m_doneCondition.notify_all();
                    }
                }
            }
        });
    }
}

ThreadPool::~ThreadPool()
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        this->m_stop = true;
    }

    this->m_condition.notify_all();

    for (std::thread& worker : this->m_workers)
    {
        if (worker.joinable())
        {
            worker.join();
        }
    }
}

void ThreadPool::WaitAll()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    this->m_doneCondition.wait(lock, [this]()
    {
        return this->m_jobs.empty() && this->m_activeWorkers == 0;
    });
}
