#ifndef THREAD_POOL_LIB_LIBRARY_H
#define THREAD_POOL_LIB_LIBRARY_H

#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <functional>

namespace thread_pool {
    template<typename T>
    class Pool {
    public:
        explicit Pool(int num_threads);

        // add work to the pool
        void addWork(const T& work);
        void addWork(T&& work);

        void addWork(const std::vector<T>& work);
        void addWork(std::vector<T>&& work);

        void setWork(const std::queue<T>& work);
        void setWork(std::queue<T>&& work);

        void clearWork();

        // let the workers do all the work
        template<typename F, typename... A>
        void run(F&&, A&&... args);

    protected:
        template<typename F, typename... A>
        void doWork(F&&function, A&&... args);

        std::vector<std::thread> threads_;
        int num_workers_;
        std::queue<T> work_;
        std::mutex work_m_;
    };

    template<typename T>
    Pool<T>::Pool(int num_threads) : num_workers_(num_threads) {
        threads_.resize(num_workers_);
    }

    template<typename T>
    void Pool<T>::Pool::addWork(const T& work) {
        std::lock_guard<std::mutex> lock(work_m_);
        work_.push(work);
    }

    template<typename T>
    void Pool<T>::Pool::addWork(T&& work) {
        std::lock_guard<std::mutex> lock(work_m_);
        work_.push(std::move(work));
    }

    template<typename T>
    void Pool<T>::addWork(const std::vector<T>& work) {
        std::lock_guard<std::mutex> lock(work_m_);
        for (const auto& element : work) {
            work_.push(element);
        }
    }

    template<typename T>
    void Pool<T>::addWork(std::vector<T>&& work) {
        std::lock_guard<std::mutex> lock(work_m_);
        for (const auto& element : work) {
            work_.push(std::move(element));
        }
    }

    template<typename T>
    void Pool<T>::setWork(const std::queue<T>& work) {
        std::lock_guard<std::mutex> lock(work_m_);
        work_ = work;
    }
    template<typename T>
    void Pool<T>::setWork(std::queue<T>&& work) {
        std::lock_guard<std::mutex> lock(work_m_);
        work_ = std::move(work);
    }

    template<typename T>
    void Pool<T>::clearWork() {
        std::lock_guard<std::mutex> lock(work_m_);
        std::queue<T> empty;
        std::swap(work_, empty);
    }

    template <typename T>
    template<typename F, typename... A>
    void Pool<T>::run(F&& function, A&&... args) {
        while(!work_.empty()) {
            const int tasks = std::min(num_workers_, int(work_.size()));
            for (int i=0; i<tasks; ++i) {
                    threads_[i] = std::thread(&Pool<T>::doWork<F, A...>, this, std::ref(function),
                                              std::forward<A>(args)...);
            }
            for (int i=0; i<tasks; ++i) {
                threads_[i].join();
            }
        }
    }

    template<typename T>
    template<typename F, typename... A>
    void Pool<T>::doWork(F&& function, A&&... args) {
        work_m_.lock();
        if (!work_.empty()) {
            T work = std::move(work_.front());
            work_.pop();
            work_m_.unlock();

            function(work, std::forward<A>(args)...);
            doWork(std::forward<F>(function), std::forward<A>(args)...);
        }
        else {
            work_m_.unlock();
        }
    }
}

#endif