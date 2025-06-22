/**
 * File: thread-pool.cc
 * --------------------
 * Presents the implementation of the ThreadPool class.
 */

#include "thread-pool.h"
using namespace std;

ThreadPool::ThreadPool(size_t numThreads) : wts(numThreads), done(false) {
    dt = thread([this]{ dispatcher(); });

    for (size_t i = 0; i < numThreads; ++i) {
        wts[i].ts = thread([this, i]{ worker(i); });
    }
}


void ThreadPool::schedule(const function<void(void)>& thunk) {
    {
        lock_guard<mutex> lk(queueLock);   
        taskQueue.push(thunk);              
    }
    {
        lock_guard<mutex> lg(tasksMutex);  
        activeTasks++;                        
    }

    queueNotEmpty.signal(); 
}

void ThreadPool::dispatcher() {
    while (true) {
        queueNotEmpty.wait();  

        {
            lock_guard<mutex> lk(queueLock);
            if (done && taskQueue.empty()) break;  
        }

        
        size_t freeWorker = SIZE_MAX;
        for (size_t i = 0; i < wts.size(); ++i) {
            if (wts[i].available) {
                freeWorker = i;
                break;
            }
        }

       
        if (freeWorker == SIZE_MAX) {
            queueNotEmpty.signal();   
            continue;                 
        }

        function<void(void)> nextTask;
        {
            lock_guard<mutex> lk(queueLock);
            if (!taskQueue.empty()) {
                nextTask = taskQueue.front();
                taskQueue.pop();
            } else {
                continue;  
            }
        }


        wts[freeWorker].available = false;
        wts[freeWorker].thunk     = nextTask;
        wts[freeWorker].ready.signal();  
    }
    
}


void ThreadPool::worker(int id) {
    while (true) {
        wts[id].ready.wait(); 

        if (done) break; 

        wts[id].thunk();

        wts[id].available = true;

        bool wasLast = false;
        {
            lock_guard<mutex> lk(tasksMutex);
            activeTasks--;
            if (activeTasks == 0) {
                wasLast = true; 
            }
        }

        if (wasLast) {
            allTasksDone.notify_all();
        }
    }
}


void ThreadPool::wait() {
    unique_lock<mutex> lk(tasksMutex);
    while (activeTasks > 0) {
        allTasksDone.wait(lk);
    }
}

ThreadPool::~ThreadPool() {
    wait();          

    done = true;      

    queueNotEmpty.signal(); 

   
    for (size_t i = 0; i < wts.size(); ++i) {
        wts[i].ready.signal();
    }

    
    if (dt.joinable()) dt.join();
    for (size_t i = 0; i < wts.size(); ++i) {
        if (wts[i].ts.joinable()) {
            wts[i].ts.join();
        }
    }
}
