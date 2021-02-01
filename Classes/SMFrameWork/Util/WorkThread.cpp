//
//  WorkThread.cpp
//  SMFrameWork
//
//  Created by SteveMac on 2018. 7. 9..
//

#include "WorkThread.h"
#include <functional>

// thread 정의

// 기본은 한개 쓰레드..
#define DEFAULT_POOL_SIZE   (1)

// 내가 진짜 쓰레드다.
class WorkThread::ThreadPool
{
public:
    ThreadPool(bool detach, int threadCount = DEFAULT_POOL_SIZE) :
    _detach(detach)
    , _running(true)
    {
        for (int i=0; i<threadCount; i++) {
            _workers.emplace_back(std::thread(std::bind(&WorkThread::ThreadPool::doWork, this)));
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
    
    void interrupt()
    {
        _running = false;
        
        std::unique_lock<std::mutex> lock(_mutex);
        _cond.notify_all();
    }
    
    void addWork(const std::function<void()>& work)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        
        _queue.emplace(work);
        _cond.notify_one();
    }
    
private:
    // 얘가 일하는 넘...
    void doWork()
    {
        while (true) {
            // 열라 뺑뺑이
            
            std::function<void()> work = nullptr;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                if (!_running) {
                    return;
                }
                
                if (!_queue.empty()) {
                    // 큐에 쌓인게 있으면 작업
                    work = std::move(_queue.front());
                    _queue.pop();
                } else {
                    // 큐에 없으면 대기
                    _cond.wait(lock);
                    if (!_running) {
                        // 풀렸는데 실행중이 아니면 쓰레드를 빠져나간다.
                        break;
                    }
                    // 일할지 다시 체크
                    continue;
                }
            }
            
            // 작업자가 있으면 일해라
            if (work) {
                work();
            }
        }
    }
    
private:
    std::mutex _mutex;
    
    std::condition_variable _cond;
    
    // thread worker
    std::vector<std::thread> _workers;
    
    // thread queue
    std::queue<std::function<void()>> _queue;
    
    bool _detach;
    
    bool _running;
};


// Work Thread Task

#define __CHECK_THREAD_INTERRUPTED__  { std::this_thread::sleep_for(std::chrono::milliseconds(1)); if (!_running) break; }

std::shared_ptr<WorkThread::WorkTask> WorkThread::WorkTask::createTaskWithTarget(WorkThreadProtocol *target)
{
    std::shared_ptr<WorkTask> task = std::make_shared<WorkTask>();
    
    task->_target = target;
    task->_workThreadTargetAlive = target->_workThreadTargetAlive;
    task->_workThread = &WorkThread::getInstance();
    
    return task;
}

std::shared_ptr<WorkThread::WorkTask> WorkThread::WorkTask::createTaskWithCallback(JobFinishCallback &callback)
{
    std::shared_ptr<WorkTask> task = std::make_shared<WorkTask>();
    task->_jobFinishCallback = callback;
    // 이거 안씀.
    return task;
}

WorkThread::WorkTask::WorkTask() :
_running(true), _function(nullptr), _cacheEntry(nullptr)
{
    
}

WorkThread::WorkTask::~WorkTask()
{
    CC_SAFE_DELETE(_function);
    _cacheEntry.reset();
}

bool WorkThread::WorkTask::init(ToDoFunction *function, const int tag)
{
    _function = function;
    _function->setTask(this);
    _tag = tag;
    
    if (_function->onBeforeDoJob()) {
        _workThread->handleState(shared_from_this(), WorkThread::State::INIT_SUCCESS);
        return true;
    } else {
        _workThread->handleState(shared_from_this(), WorkThread::State::INIT_FAIL);
        return false;
    }
}


// 한개의 일을 하는 쓰레드 함수
void WorkThread::WorkTask::doWorkThread()
{
    do {
        __CHECK_THREAD_INTERRUPTED__;
        _cacheEntry.reset();
        if (_function->onJobInBackground()) {
            __CHECK_THREAD_INTERRUPTED__;
            _workThread->handleState(shared_from_this(), WorkThread::State::PROCESS_SUCCESS);
            return;
        }
        
        __CHECK_THREAD_INTERRUPTED__;
    } while (0);
    
    _workThread->handleState(shared_from_this(), WorkThread::State::PROCESS_FAIL);
    _cacheEntry.reset();
}

void WorkThread::WorkTask::interrupt()
{
    _running = false;
    if (_function) {
        _function->interrupt();
    }
}

void WorkThread::WorkTask::onProgress(int idx, int total, float progress, std::string desc)
{
    _workThread->handleState(shared_from_this(), WorkThread::State::PROGRESS, idx, total, progress, desc);
}


// Work Thread

WorkThread::WorkThread() : _workThreadPool(nullptr)
{
    init();
}

WorkThread::~WorkThread()
{
    _workThreadPool->interrupt();
    CC_SAFE_DELETE(_workThreadPool);
}

void WorkThread::init()
{
    _workThreadPool = new ThreadPool(true, 1);
}

void WorkThread::doWork(WorkThreadProtocol *target, ToDoFunction *function, const int tag)
{
    CCASSERT(target != nullptr && function != nullptr, "INVALID PARAM");
    
    std::shared_ptr<WorkTask> task = WorkTask::createTaskWithTarget(target);
    task->init(function, tag);
}

void WorkThread::doWork(JobFinishCallback &callback, ToDoFunction *function, const int tag)
{
    // 이거 안쓸거임..
    CCASSERT(function != nullptr, "INVALID PARAM");
    std::shared_ptr<WorkTask> task = WorkTask::createTaskWithCallback(callback);
    task->init(function, tag);
}

void WorkThread::stopWork(WorkThreadProtocol *target)
{
    if (target) {
        target->resetWorkThread();
    }
}

void WorkThread::stopWork(int tag)
{
    // 이거 안쓸거임...
}

void WorkThread::handleState(std::shared_ptr<WorkTask> task_ptr, WorkThread::State state, int intParam1, int intParam2, float floatParam, std::string desc)
{
    switch (state) {
        case State::INIT_SUCCESS:
        {
            // 한개의 쓰레드를 생성... 실행할 함수를 바인딩한다.
            auto threadFunc = std::bind(&WorkTask::doWorkThread, task_ptr);
            // Thread pool에 넣는다.
            _workThreadPool->addWork(threadFunc);
        }
            break;
        case State::INIT_FAIL:
        {
            // 지금 메인 쓰레드이므로 그냥 호출 해도 된다.
            if (task_ptr->isTargetAlive()) {
                task_ptr->getTarget()->onJobFinish(task_ptr->getTag(), false, nullptr);
                task_ptr->getTarget()->removeWorkThreadTask(task_ptr);
            }
        }
            break;
        case State::PROCESS_SUCCESS:
        {
            std::lock_guard<std::mutex> guard(_mutex);
            
            // 지금 다른 쓰레드에서 돌고 있으므로 메인 쓰레드에서 호출 하도록 한다.
            if (task_ptr->isTargetAlive()) {
                cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread([task_ptr]{
                    // 살아 있는지 한번더 체크
                    if (task_ptr->isTargetAlive()) {
                        auto function = task_ptr->getToDoFunction();
                        Intent * intent = function->onAfterDoneJob();
                        
                        task_ptr->getTarget()->onJobFinish(task_ptr->getTag(), true, intent);
//                        task_ptr->getTarget()->onJobFinish(task_ptr->getTag(), true, function->getParam());
                        task_ptr->getTarget()->removeWorkThreadTask(task_ptr);
                    }
                });
            }
        }
            break;
        case State::PROCESS_FAIL:
        {
            std::lock_guard<std::mutex> guard(_mutex);
         
            // 지금 다른 쓰레드에서 돌고 있으므로 메인 쓰레드에서 호출 하도록 한다.
            if (task_ptr->isTargetAlive()) {
                cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread([task_ptr]{
                    if (task_ptr->isTargetAlive()) {
                        task_ptr->getTarget()->removeWorkThreadTask(task_ptr);
                    }
                });
            }
        }
            break;
        case State::PROGRESS:
        {
            std::lock_guard<std::mutex> guard(_mutex);
            
            // progress가 필요 할때..
            if (task_ptr->isTargetAlive()) {
                cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread([task_ptr, intParam1, intParam2, floatParam, desc]{
                    if (task_ptr->isTargetAlive()) {
                        task_ptr->getTarget()->onJobProgress(task_ptr->getTag(), intParam1, intParam2, floatParam, desc);
                    }
                });
            }
        }
            break;
            
        default:
            break;
    }
}



// protocol

void WorkThreadProtocol::resetWorkThread()
{
    for (auto iter = _workThreadTask.begin(); iter !=  _workThreadTask.end(); ++iter) {
        if (!iter->expired()) {
            auto task = iter->lock();
            if (task!=nullptr && task->isRunning()) {
                task->interrupt();
            }
        }
        iter->reset();
    }
    
    _workThreadTask.clear();
    
    _workThreadTargetAlive.reset();
    _workThreadTargetAlive = std::make_shared<bool>(true);
}

void WorkThreadProtocol::removeWorkThreadTask(std::shared_ptr<WorkThread::WorkTask> task)
{
    for (auto iter=_workThreadTask.begin(); iter!=_workThreadTask.end();) {
        auto t= iter->lock();
        
        if (iter->expired()) {
            // expire 된거면 삭제
            iter = _workThreadTask.erase(iter);
        } else if (task && t && task==t) {
            // 현재 task면 interrupt후 삭제
            task->interrupt();
            iter->reset();
            iter = _workThreadTask.erase(iter);
        } else {
            ++iter;
        }
    }
}

bool WorkThreadProtocol::addWorkThreadTask(std::shared_ptr<WorkThread::WorkTask> task)
{
    for (auto iter=_workThreadTask.begin(); iter!=_workThreadTask.end();) {
        auto t = iter->lock();
        
        if (iter->expired()) {
            // expire 된거면 삭제
            iter = _workThreadTask.erase(iter);
        } else if (task && t && task==t) {
            // 현재 task면 그냥 return
            return false;
        } else {
            ++iter;
        }
    }
    
    // 새로운거면 추가
    _workThreadTask.emplace_back(task);
    return true;
}

// To Do Function

ToDoFunction::ToDoFunction() : _task(nullptr)
, _param(nullptr)
{
    initParam();
}

ToDoFunction::~ToDoFunction()
{
    CC_SAFE_RELEASE(_param);
}

void ToDoFunction::setTask(WorkThread::WorkTask *task)
{
    _task = task;
}

Intent* ToDoFunction::initParam()
{
    if (_param == nullptr) {
        _param = Intent::create();
        _param->retain();
    }
    
    return _param;
}

void ToDoFunction::onProgress(int idx, int total, float progress, std::string desc)
{
    _task->onProgress(idx, total, progress, desc);
}
