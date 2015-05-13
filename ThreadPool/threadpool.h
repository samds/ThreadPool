//
//  threadpool.h
//  ThreadPool
//
//  Created by samuel.
//  Copyright (c) 2014 SDS. All rights reserved.
//

#ifndef ThreadPool_threadpool_h
#define ThreadPool_threadpool_h


#include <iostream>
#include <thread>
#include <vector>
#include <queue>

/*
 * A basic thread-pool using new C++11 features.
 */
class ThreadPool {
    
    std::mutex mtx;
    std::condition_variable cv;
    bool exitRequested;
    
    std::vector<std::thread> pool;
    std::queue< std::function<void()> > tasks;
    
public:
    typedef decltype(tasks)::value_type     value_type;
    typedef decltype(tasks)::size_type      size_type;
    
private:
    
    /*
     * Initializes a worker thread.
     */
    void initWorker() {
        while(true)
        {
            std::function<void()> task;
            
            {
                std::unique_lock<std::mutex> lock(mtx);
                
                if (!exitRequested && !hasWork()) {
                    std::cout << "Thread " << std::this_thread::get_id() << " is waiting." << std::endl;
                }
                
                // Wait only if no task is available
                cv.wait(lock,[this]{ return exitRequested || hasWork(); });
                
                // If all tasks completed, exits
                if( isDone() ) {
                    // std::cout << "Thread " << std::this_thread::get_id() << " done." << std::endl;
                    return;
                }
                
                // Can be used instead of the wait() but is less efficient.
                // if (!hasWork()) {
                //  std::this_thread::yield();
                //  continue;
                // }
                
                std::cout << "Thread " << std::this_thread::get_id() << " work in progress." << std::endl;
                
                // Otherwise, picks up a task in the pool
                task = std::move(this->tasks.front());
                // Removes the task from the pool
                this->tasks.pop();
                
            } // unlock the mutex on unique_lock's destruction
            
            
            // Does the task!
            task();
        } // end while()
    }
    
    /*
     * Returns true if a task is available.
     */
    bool hasWork() const {
        return !this->tasks.empty();
    }
    
    /*
     * Returns if job is done.
     *
     * The work is done when the tasks are completed or
     * when user exits the program.
     */
    bool isDone() const {
        return exitRequested && this->tasks.empty();
    }
    
public:
    
    /*
     * Creates a thread pool
     *
     * @param n     number of workers
     */
    explicit ThreadPool(size_t n) : exitRequested(false) {
        // Construct and insert threads
        for(size_t i = 0 ; i<n ; ++i) {
            pool.emplace_back(&ThreadPool::initWorker,this);
        }
    }
    
    /*
     * Destructor
     *
     * Joins all thread (wait that all tasks are completed).
     */
    virtual ~ThreadPool()
    {
        // Sets stop to true
        {
            std::unique_lock<std::mutex> lock(mtx);
            exitRequested = true;
        } // Unlock the mutex associated to the unique_lock
        // Can be replaced by
        // mtx.lock();
        // exitRequested = true;
        // mtx.unlock();
        
        
        // Notify all threads (in the pool) that the condition changed
        cv.notify_all();
        
        // Waits that all threads fnish their work
        for(std::thread &th: pool) {
            th.join();
        }
    }
    
    
    /*
     * Inserts a new worker in the pool
     */
    void push(value_type&& func)
    {
        std::unique_lock<std::mutex> lock(mtx);
        tasks.push(func);
        cv.notify_one();
    }
    
    /*
     * Inserts a new worker in the pool
     */
    void push(const value_type& func)
    {
        std::unique_lock<std::mutex> lock(mtx);
        tasks.push(func);
        cv.notify_one();
    }
    
    /*
     * Constructs and inserts a new worker in the pool
     */
    template<class F, class... Args>
    void emplace(F&& f, Args&&... args)
    {
        auto func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        std::unique_lock<std::mutex> lock(mtx);
        tasks.emplace(func);
        cv.notify_one();
    }
    
};


#endif
