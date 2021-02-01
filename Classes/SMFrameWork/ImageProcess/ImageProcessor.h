//
//  ImageProcessor.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 12. 5..
//
// Image 효과 전처리기...downloader 처럼 thread로 동작

#ifndef ImageProcessor_h
#define ImageProcessor_h

//#include <2d/CCSprite.h>
#include "../Base/Intent.h"
#include <cocos2d.h>
#include <memory>
#include <mutex>

class ImageProcessFunction;
class ImageProcessProtocol;

class ImageProcessor
{
public:
    static ImageProcessor& getInstance() {
        static ImageProcessor instance;
        return instance;
    }
    
    void executeImageProcess(ImageProcessProtocol* target, cocos2d::Node* node, ImageProcessFunction* function, const int tag=0);
    
    void cancelImageProcess(ImageProcessProtocol* target);
    
    class ImageProcessTask;

protected:
    
    class ThreadPool;

    enum class  State {
        INIT_SUCCESS,
        INIT_FAILED,
        
        PROCESS_SUCCESS,
        PROCESS_FAILED,
        
        PROGRESS,
    };
    
    void handleState(std::shared_ptr<ImageProcessTask> task_ptr, State state, int intParam=0, float floatParam=0);
    
//    void handleItem(std::shared_ptr<ImageProcessTask> task_ptr, State state);
    
protected:
    ImageProcessor();
    virtual ~ImageProcessor();
    void init();
    
    
private:
    std::mutex _mutex;
    
    ThreadPool* _processThreadPool;
    friend class ImageProcessProtocol;
};

class ImageProcessProtocol
{
public:
    ImageProcessProtocol() {
        _ImageProcessTargetAlive = std::make_shared<bool>(true);
    }
    
    virtual ~ImageProcessProtocol()
    {
        resetImageProcess();
    }
    
    // 이것 저것 이미지처리
    virtual void onImageProcessComplete(const int tag, const bool success, cocos2d::Sprite* sprite, Intent* intent) {}
    // 화면 캡쳐일때
    virtual void onImageCaptureComplete(const int tag, cocos2d::Texture2D* texture, uint8_t* data, const cocos2d::Size& size, const int bpp){}
    
    // progressive
    virtual void onImageProcessProgress(const int tag, const float progress) {}
    
    // reset
    void resetImageProcess();
    
protected:
    void removeImageProcessTask(std::shared_ptr<ImageProcessor::ImageProcessTask> task_ptr);
    
    bool addImageProcessTask(std::shared_ptr<ImageProcessor::ImageProcessTask> task_ptr);
    
    std::vector<std::weak_ptr<ImageProcessor::ImageProcessTask>> _imageProcessTasks;
    
    std::shared_ptr<bool> _ImageProcessTargetAlive;
    
    friend class ImageProcessor;
};

// ImageProcessTask
class ImageProcessor::ImageProcessTask : public std::enable_shared_from_this<ImageProcessor::ImageProcessTask>
{
protected:
    static std::shared_ptr<ImageProcessTask> createTaskForTarget(ImageProcessProtocol* target);
    bool init(cocos2d::Node* node, ImageProcessFunction* function, const int tag);
    void interrupt();
//    inline void interrupt() {_running = false; if (_function) {_function->interrupt();}}
    inline bool isRunning() {return _running;}
    inline bool isTargetAlive() {return !_imageProcessTargetAlive.expired();}
    int getTag() {return _tag;}
    ImageProcessProtocol* getTarget() {return _target;}
    ImageProcessFunction* getProcessFunction() {return _function;}
    void onProgress(float progress);
    
public:
    void procImageProcessThread();
    
public:
    ImageProcessTask();
    virtual ~ImageProcessTask();
    
private:
    bool _running;
    int _tag;
    std::weak_ptr<bool> _imageProcessTargetAlive;
    ImageProcessor* _processor;
    ImageProcessProtocol* _target;
    ImageProcessFunction* _function;
    
    friend class ImageProcessor;
    friend class ImageProcessProtocol;
    friend ImageProcessFunction;
};



#endif /* ImageProcessor_h */
