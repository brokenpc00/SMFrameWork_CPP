//
//  WorkThread.hpp
//  SMFrameWork
//
//  Created by SteveMac on 2018. 7. 9..
//

#ifndef WorkThread_h
#define WorkThread_h

#include "../Base/Intent.h"
#include <cocos2d.h>
#include <memory>
#include <mutex>
#include "MemoryCache.h"

// 실제 일하는 넘. 하늘 일마다 다름
class ToDoFunction;

// 일 끝나면 돌려주고, thread 관리 하는 넘.
class WorkThreadProtocol;

typedef std::function<void(bool success, Intent* intent)> JobFinishCallback;

// 이넘은 그냥 thread 포맷... 맨날 똑같음.
class WorkThread {
public:
    static WorkThread& getInstance() {
        static WorkThread instance;
        return instance;
    }
    
    // 일해라 그리고 target한테 결과를 돌려줘~ (intent는 일하는데 필요한 재료)
    void doWork(WorkThreadProtocol* target, ToDoFunction* function, const int tag=0);
    
    void doWork(JobFinishCallback& callback, ToDoFunction* function, const int tag=0);
    
    // 고만해~
    void stopWork(WorkThreadProtocol* target);
    
    void stopWork(int tag);
    
    // 일하는 단위 개체... alive check 등...
    class WorkTask;
    
protected:
    // 쓰레드로 돌릴거임
    class ThreadPool;
    
    enum class State {
        INIT_SUCCESS,
        INIT_FAIL,
        
        PROCESS_SUCCESS,
        PROCESS_FAIL,
        
        PROGRESS,
    };
    
    void handleState(std::shared_ptr<WorkTask> task_ptr, State state, int intParam1=0, int intParam2=0, float floatParam=0, std::string desc="");
    
protected:
    WorkThread();
    virtual ~WorkThread();
    
    void init();
    
private:
    std::mutex _mutex;
    
    ThreadPool* _workThreadPool;
    
    friend class WorkThreadProtocol;
};

class WorkThreadProtocol
{
public:
    WorkThreadProtocol() {
        _workThreadTargetAlive = std::make_shared<bool>(true);
    }
    
    virtual ~WorkThreadProtocol() {
        resetWorkThread();
    }
    
    // 일 끝났음.
    virtual void onJobFinish(const int tag, const bool success, Intent* intent) {}
    
    virtual void onJobProgress(const int tag, const int idx, const int total, const float progress, std::string desc) {}
    
    // 리셋해줘...
    void resetWorkThread();

protected:
    // 작업 삭제
    void removeWorkThreadTask(std::shared_ptr<WorkThread::WorkTask> task);
    
    // 작업 추가
    bool addWorkThreadTask(std::shared_ptr<WorkThread::WorkTask> task);
    
    // 현재 작업 목록
    std::vector<std::weak_ptr<WorkThread::WorkTask>> _workThreadTask;
    
    std::shared_ptr<bool> _workThreadTargetAlive;
    
    friend class WorkThread;
    
    
};

class WorkThread::WorkTask : public std::enable_shared_from_this<WorkThread::WorkTask>
{
public:
    static std::shared_ptr<WorkTask> createTaskWithTarget(WorkThreadProtocol* target);
    static std::shared_ptr<WorkTask> createTaskWithCallback(JobFinishCallback& callback);
    
public:
    bool init(ToDoFunction* function, const int tag);
    
    void interrupt();
    
    // 돌고 있냐?
    inline bool isRunning() {return _running;}
    // 살아 있냐?
    inline bool isTargetAlive() {return !_workThreadTargetAlive.expired();}
    
    int getTag() {return _tag;}
    
    WorkThreadProtocol* getTarget() {return _target;}
    
    ToDoFunction* getToDoFunction() {return _function;}
    
    std::shared_ptr<MemoryCacheEntry> getMemoryCacheEntry() {return _cacheEntry;}
    void setMemoryCacheEntry(std::shared_ptr<MemoryCacheEntry>* cacheEntry) {_cacheEntry = *cacheEntry;}

//    void onProgress(float progress);
    
    void onProgress(int idx, int total, float progress, std::string desc);
    
public:
    // thread function.. 이 쓰레드 처리 루틴의 가장 main
    void doWorkThread();
    
public:
    WorkTask();
    virtual ~WorkTask();
    
private:
    // 돌고 있냐?
    bool _running;
    int _tag;
    
    // 일시킨 넘이 살아 있는지
    std::weak_ptr<bool> _workThreadTargetAlive;
    
    std::shared_ptr<MemoryCacheEntry> _cacheEntry;

    WorkThread* _workThread;
    WorkThreadProtocol* _target;
    ToDoFunction* _function;
    JobFinishCallback _jobFinishCallback;
    
    friend WorkThread;
    friend WorkThreadProtocol;
    friend ToDoFunction;
};

// 일 시킬 단위
class ToDoFunction
{
public:
    ToDoFunction();
    virtual ~ToDoFunction();
    
    // before -> background -> after done
    // 실제 호출은 task가 한다... 얘는 각 단계별로 뭘할지 정의만 한다.
    
    virtual bool onBeforeDoJob() {return true;}
    
    virtual bool onJobInBackground() {return true;}
    
    virtual Intent* onAfterDoneJob() {return _param;}
    
    Intent* getParam() {return _param;}
    
    WorkThread::WorkTask* getTask() {return _task;}
    
protected:
    void setTask(WorkThread::WorkTask* task);
    
    Intent* initParam();
    
protected:
    void interrupt() {_interrupt = true;}
    bool isInterrupted() {return _interrupt;}
    void onProgress(int idx, int total, float progress, std::string desc);
    
    
private:
    Intent* _param;
    
    bool _interrupt;
    
    WorkThread::WorkTask* _task;
    
    friend class WorkThread;
    
};

#endif /* WorkThread_h */
