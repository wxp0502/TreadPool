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
public:
    explicit ThreadPool(size_t thread_amount) : _stop(false) {
        for (size_t i = 0; i < thread_amount; ++i) {
            _thread.emplace_back(
                    [this] {
                        while (!_stop) {
                            std::function<void()> task;

                            std::unique_lock<std::mutex> lock(this->_mutex);
                            this->_cv.wait(lock,
                                           [this] { return !this->_tasks.empty(); });
                            if (this->_tasks.empty())
                                return;
                            task = std::move(this->_tasks.front());
                            this->_tasks.pop();

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
        auto task = std::make_shared<std::packaged_task<return_type()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        std::future<return_type> res = task->get_future();
        std::unique_lock<std::mutex> lock(_mutex);
        if (_stop)
            throw std::runtime_error("Thread has stopped");
        _tasks.emplace([task]() { (*task)(); });
        _cv.notify_one();
        return res;
    }

    ~ThreadPool() {
        std::unique_lock<std::mutex> lock(_mutex);
        _stop = true;
        _cv.notify_all();
        for (auto &t: _thread)
            t.join();
    }

private:
    // synchronization
    std::mutex _mutex;
    std::condition_variable _cv;
    bool _stop;

    // keep track of threads
    std::vector<std::thread> _thread;
    // the task queue
    std::queue<std::function<void()>> _tasks;

};


#endif //TREADPOOL_THREADPOOL_H
