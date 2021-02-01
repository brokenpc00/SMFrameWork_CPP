//
//  ImageProcessor.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 12. 5..
//
//

#include "ImageProcessor.h"
#include "ImageProcessFunction.h"
#include <functional>
#include <queue>
#include <condition_variable>


// download와 달리 일단 하나씩만 하자...
#define DEFAULT_POOL_SIZE   (1)

class ImageProcessor::ThreadPool
{
public:
    // 동시에 여러개 처리를 원하면 threadCount를 늘려주자.
    ThreadPool(bool detach, int threadCount = DEFAULT_POOL_SIZE) :
    _detach(detach),
    _running(true)
    {
        for (int i=0; i<threadCount; i++) {
            _workers.emplace_back(std::thread(std::bind(&ImageProcessor::ThreadPool::threadFunc, this)));
            if (_detach) {
                _workers[i].detach();
            }
        }
    }
    
    ~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _running = false;
            _cond.notify_all();
        }
        
        if (!_detach) {
            for (auto&& worker : _workers) {
                worker.join();
            }
        }
        _workers.clear();
    }
    
    void interrupt()
    {
        _running = false;
        {
            std::unique_lock<std::mutex> lock(_mutex);
            // 전부통지
            _cond.notify_all();
        }
    }
    
    void addTask(const std::function<void()> &task)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        //새 작업을 큐에 넣고
        _queue.emplace(task);
        // 한개 통지
        _cond.notify_one();
    }
    
private:
    void threadFunc()
    {
        while (true) {
            std::function<void()> task = nullptr;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                if (!_running) {
                    break;
                }

                if (!_queue.empty()) {
                    // 할게 있으면 앞에것을 끄집어 낸다.
                    task = std::move(_queue.front());
                    _queue.pop();
                } else {
                    // 할게 없으면 기둘..
                    _cond.wait(lock);
                    if (!_running) {
                        break;
                    }
                    continue;
                }
            }
            // 일하자!
            if (task) {
                task();
            }
        }
    }
    
private:
    bool _detach;
    bool _running;
    
    std::queue<std::function<void()>> _queue;
    std::vector<std::thread> _workers;
    std::condition_variable _cond;
    std::mutex _mutex;
};




#define __CHECK_THREAD_INTERRUPTED__  { std::this_thread::sleep_for(std::chrono::milliseconds(1)); if (!_running) break; }
std::shared_ptr<ImageProcessor::ImageProcessTask> ImageProcessor::ImageProcessTask::createTaskForTarget(ImageProcessProtocol *target)
{
    std::shared_ptr<ImageProcessTask> task = std::make_shared<ImageProcessTask>();
    
    task->_target = target;
    task->_imageProcessTargetAlive = target->_ImageProcessTargetAlive;
    task->_processor = &ImageProcessor::getInstance();
    
    return task;
}

ImageProcessor::ImageProcessTask::ImageProcessTask() :
_running(true),
_function(nullptr)
{
    
}

ImageProcessor::ImageProcessTask::~ImageProcessTask()
{
    CC_SAFE_DELETE(_function);
}

bool ImageProcessor::ImageProcessTask::init(cocos2d::Node *node, ImageProcessFunction *function, const int tag)
{
    _function = function;
    _function->setTask(this);
    _tag = tag;
    
    if (_function->onPreProcess(node)) {
        _processor->handleState(shared_from_this(), ImageProcessor::State::INIT_SUCCESS);
        return true;
    } else {
        _processor->handleState(shared_from_this(), ImageProcessor::State::INIT_FAILED);
        return false;
    }
}

void ImageProcessor::ImageProcessTask::procImageProcessThread()
{
    do {
        __CHECK_THREAD_INTERRUPTED__;
        if (_function->onProcessInBackground()) {
            __CHECK_THREAD_INTERRUPTED__;
            // 성공
            _processor->handleState(shared_from_this(), ImageProcessor::State::PROCESS_SUCCESS);
            return;
        }
        
        __CHECK_THREAD_INTERRUPTED__;
        
    } while (0);
    
    // 실패
    _processor->handleState(shared_from_this(), ImageProcessor::State::PROCESS_FAILED);
}

void ImageProcessor::ImageProcessTask::onProgress(float progress)
{
    _processor->handleState(shared_from_this(), ImageProcessor::State::PROGRESS, 0, progress);
}


void ImageProcessor::ImageProcessTask::interrupt()
{
    _running = false;
    if (_function)
        _function->interrupt();
}




// Image Processor

ImageProcessor::ImageProcessor() :
_processThreadPool(nullptr)
{
    init();
}

ImageProcessor::~ImageProcessor()
{
    _processThreadPool->interrupt();
    CC_SAFE_DELETE(_processThreadPool);
}

void ImageProcessor::init()
{
    // 한개로 처리하고 종료
    _processThreadPool = new ThreadPool(true, 1);
}

void ImageProcessor::executeImageProcess(ImageProcessProtocol *target, cocos2d::Node *node, ImageProcessFunction *function, const int tag)
{
    CCASSERT(target != nullptr && function != nullptr, "INVALID PARAM");

    std::shared_ptr<ImageProcessTask> task = ImageProcessTask::createTaskForTarget(target);
    task->init(node, function, tag);
}

void ImageProcessor::cancelImageProcess(ImageProcessProtocol *target)
{
    if (target) {
        target->resetImageProcess();
    }
}

void ImageProcessor::handleState(std::shared_ptr<ImageProcessTask> task_ptr, ImageProcessor::State state, int intParam, float floatParam)
{
    switch (state) {
        case State::INIT_SUCCESS:
        {
            // lock guard mutex
//            std::lock_guard<std::mutex> guard(_mutex);
            
            if (task_ptr->isTargetAlive()) {
                if (task_ptr->getProcessFunction()->isCaptureOnly()) {
                    // 캡쳐만 하는거면 캡쳐하고 종료
                    auto sprite = cocos2d::Sprite::createWithTexture(task_ptr->getProcessFunction()->getCapturedTexture());
                    task_ptr->getTarget()->onImageProcessComplete(task_ptr->getTag(), true, sprite, task_ptr->getProcessFunction()->getParam());
                    task_ptr->getTarget()->removeImageProcessTask(task_ptr);
                    
                    break;
                } else {
                    task_ptr->getTarget()->onImageCaptureComplete(task_ptr->getTag(), task_ptr->getProcessFunction()->getCapturedTexture(), task_ptr->getProcessFunction()->getInputData(), task_ptr->getProcessFunction()->getInputSize(), task_ptr->getProcessFunction()->getInputBpp());
                }
            }
            
            auto threadFunc = std::bind(&ImageProcessTask::procImageProcessThread, task_ptr);
            _processThreadPool->addTask(threadFunc);
        }
            break;
        case State::INIT_FAILED:
        {
//            std::lock_guard<std::mutex> guard(_mutex);
            
            if (task_ptr->isTargetAlive()) {
                task_ptr->getTarget()->onImageProcessComplete(task_ptr->getTag(), false, nullptr, nullptr);
                task_ptr->getTarget()->removeImageProcessTask(task_ptr);
            }
        }
            break;
        case State::PROCESS_FAILED:
        {
            std::lock_guard<std::mutex> guard(_mutex);
            
            // fail
            if (task_ptr->isTargetAlive()) {
                auto scheduler = cocos2d::Director::getInstance()->getScheduler();
                scheduler->performFunctionInCocosThread([task_ptr]{
                    // cocos thread에서 한번더 체크
            if (task_ptr->isTargetAlive()) {
                task_ptr->getTarget()->onImageProcessComplete(task_ptr->getTag(), false, nullptr, nullptr);
                task_ptr->getTarget()->removeImageProcessTask(task_ptr);
            }
                });
            }
        }
            break;
        case State::PROCESS_SUCCESS:
        {
            std::lock_guard<std::mutex> guard(_mutex);
            
            if (task_ptr->isTargetAlive()) {
                // success
                auto scheduler = cocos2d::Director::getInstance()->getScheduler();
                scheduler->performFunctionInCocosThread([task_ptr]{
                    // cocos thread에서 한번더 체크
                    if (task_ptr->isTargetAlive()) {
                        auto function = task_ptr->getProcessFunction();
                        auto sprite = function->onPostProcess();
                        
                        task_ptr->getTarget()->onImageProcessComplete(task_ptr->getTag(), true, sprite, function->getParam());
                        task_ptr->getTarget()->removeImageProcessTask(task_ptr);
                    }
                });
            }
        }
            break;
        case State::PROGRESS:
        {
            std::lock_guard<std::mutex> guard(_mutex);
            
            if (task_ptr->isTargetAlive()) {
                // 완전 성공
                auto scheduler = cocos2d::Director::getInstance()->getScheduler();
                scheduler->performFunctionInCocosThread([task_ptr, floatParam] {
                    if (task_ptr->isTargetAlive()) {
                        task_ptr->getTarget()->onImageProcessProgress(task_ptr->getTag(), floatParam);
                    }
                });
            }
        }
            break;

    }
}


// image process protocol
void ImageProcessProtocol::resetImageProcess()
{
    for (auto iter=_imageProcessTasks.begin(); iter!=_imageProcessTasks.end(); ++iter) {
        if (iter->expired()) {
            // 만기된 task는 버린다.
            auto task = iter->lock();
            if (task!=nullptr && task->isRunning()) {
                task->interrupt();
            }
        }
        iter->reset();
    }
    
    // 모든 task를 clear
    _imageProcessTasks.clear();
    
    // alive flag를 새걸로 표체
    _ImageProcessTargetAlive =  std::make_shared<bool>(true);
}

void ImageProcessProtocol::removeImageProcessTask(std::shared_ptr<ImageProcessor::ImageProcessTask> task_ptr)
{
    for (auto iter=_imageProcessTasks.begin(); iter!=_imageProcessTasks.end(); ) {
        auto task = iter->lock();
        
        if (iter->expired()) {
            // 만기된 task는 버린다.
            iter = _imageProcessTasks.erase(iter);
        } else if (task_ptr && task_ptr == task) {
            // 지우려는 task 종료
            task_ptr->interrupt();
            iter->reset();
            iter = _imageProcessTasks.erase(iter);
        } else {
            // 다음 task
            ++iter;
        }
    }
}

bool ImageProcessProtocol::addImageProcessTask(std::shared_ptr<ImageProcessor::ImageProcessTask> task_ptr)
{
    // 이미 있는지 검사
    for (auto iter=_imageProcessTasks.begin(); iter!=_imageProcessTasks.end();) {
        auto task = iter->lock();
        
        if (iter->expired()) {
            // 만기된 task는 버린다.
            iter = _imageProcessTasks.erase(iter);
        } else if (task_ptr && task_ptr==task && task->isRunning()) {
            // 이미 있으면 돌고있는 task중에 있으면 리턴
            return false;
        } else {
            ++iter;
        }
    }
    
    // 없으면 새로 추가
    _imageProcessTasks.emplace_back(task_ptr);
    
    return true;
}
