//
//  ImageDownloaderThreadPool.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 16..
//
//

#ifndef ImageDownloaderThreadPool_h
#define ImageDownloaderThreadPool_h

#include <thread>
#include <memory>
#include <mutex>
#include <functional>
#include <condition_variable>
#include <queue>

//#define DEFAULT_POOL_SIZE (8)
#define DEFAULT_POOL_SIZE (4)

class ImageDownloader::ThreadPool {
public:
    ThreadPool(bool detach, int threadCount=DEFAULT_POOL_SIZE) :
    _detach(detach),
    _running(true)
    {
        for (int i=0; i<threadCount; i++) {
            _workers.emplace_back(std::thread(std::bind(&ThreadPool::threadFunc, this)));
            
            if (_detach) {
                _workers[i].detach();
            }
        }
    }
    
    ~ThreadPool()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _running = false;
        _cond.notify_all();
        
        if (!_detach) {
            for (auto&& worker : _workers) {
                worker.join();
            }
        }
        
        _workers.clear();
    }
    
    // interrupt
    void interrupt() {
        _running = false;
        
        std::unique_lock<std::mutex> lock(_mutex);
        _cond.notify_all();
    }
    
    void addTask(const std::function<void()>& task) {
        std::unique_lock<std::mutex> lock(_mutex);
        _queue.emplace(task);
        _cond.notify_one();
    }

    
private:
    void threadFunc() {
        while (true) {
            
            std::function<void()> task = nullptr;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                if (!_running) {
                    break;
                }
                
                if (!_queue.empty()) {
                    // 일 있으면
                    // 큐에서 하나 끄집어 냄.
                    task = std::move(_queue.front());
                    _queue.pop();
                } else {
                    // 일 없으면 기둘....
                    _cond.wait(lock);
                    if (!_running) {
                        break;
                    }
                    continue;
                }
            }
            
            if (task) {
                task();
            }
        
//            std::unique_lock<std::mutex> lock(_mutex);
//            if (!_running) {
//                break;
//            }
//            
//            if (!_queue.empty()) {
//                std::function<void()> task = std::move(_queue.front());
//                _queue.pop();
//                
//                if (task) {
//                    task();
//                }
//            } else {
//                _cond.wait(lock);
//                if (!_running) {
//                    break;
//                }
//            }
        }
    }
    
    std::mutex _mutex;
    std::condition_variable _cond;
    std::vector<std::thread> _workers;
    std::queue<std::function<void()>> _queue;
    bool _detach;
    bool _running;
};

#endif /* ImageDownloaderThreadPool_h */
