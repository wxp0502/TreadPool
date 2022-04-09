//
// Created by shitman on 2022/04/08.
//

#ifndef TREADPOOL_THREADPOOL_H
#define TREADPOOL_THREADPOOL_H

#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>
#include <vector>
#include <queue>

class ThreadPool {

private:
    // synchronization
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;

    // keep track of threads
    std::vector<std::thread> workers;
    // the task queue
    std::queue<std::function<void()>> tasks;


public:
    explicit ThreadPool(size_t thread_amount)
            : stop(false) {
        for (size_t i = 0; i < thread_amount; ++i) {
            workers.emplace_back(
                    [this] {
                        while (true) {
                            std::function<void()> task;
                            {   // avoid two threads try to take the same work at the same time
                                std::unique_lock<std::mutex> lock(this->queue_mutex);

                                this->condition.wait(lock,
                                                     [this] {
                                                         return this->stop || !tasks.empty();
                                                     });

                                if (this->stop && this->tasks.empty()) {
                                    return;
                                }

                                task = std::move(this->tasks.front());
                                this->tasks.pop();
                            }
                            task();
                        }
                    }
            );
        }
    };

    template<class F, class... Args>
    auto enqueue(F &&f, Args &&... args)
    -> std::future<typename std::result_of<F(Args...)>::type> {

        using return_type = typename std::result_of<F(Args...)>::type;

        // create a std::packaged_task and just wrap again this packaged task_ptr inside a std::shared_ptr
        auto task_ptr = std::make_shared<std::packaged_task<return_type()>>(
                // creates a wrapper for F with the given Args.
                std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        // the future of the packaged_task as the result
        std::future<return_type> res = task_ptr->get_future();

        {
            // ensure only one work being pushed at the same time
            std::unique_lock<std::mutex> lock(queue_mutex);

            if (stop) {
                throw std::runtime_error("Thread has stopped");
            }

            // a generic void function, considering different return types
            // Wrap packaged task_ptr into void function, declaring this wrapper_func that will execute the bound function
            std::function<void()> wrapper_func = [task_ptr]() {
                (*task_ptr)();
            };
            tasks.emplace(wrapper_func);
        }
        // Wake up one thread
        condition.notify_one();

        return res;
    }


    void shutDown() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }

        condition.notify_all();
        // join all threads
        for (std::thread &worker: workers)
            worker.join();
    }
};


#endif //TREADPOOL_THREADPOOL_H
