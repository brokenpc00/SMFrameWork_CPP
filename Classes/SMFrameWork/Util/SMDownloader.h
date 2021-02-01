//
//  SMDownloader.h
//  SMFrameWork
//
//  Created by SteveMac on 2018. 9. 7..
//

#ifndef SMDownloader_h
#define SMDownloader_h

#include "../../SMFrameWork/Util/WorkThread.h"
#include "../Base/Intent.h"
#include <base/CCRef.h>
#include <mutex>
#include <memory>
#include <string>

#include "FileManager.h"

#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
#include <external/curl/include/ios/curl/curl.h>
#elif CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
#include <external/curl/include/android/curl/curl.h>
#elif CC_TARGET_PLATFORM == CC_PLATFORM_MAC
#include <external/curl/include/mac/curl/curl.h>
#elif CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
#include <external/curl/include/win32/curl/curl.h>
#endif


class DownloadInfo : public cocos2d::Ref {
public:
    std::vector<int8_t> data;
    bool bSave;
    std::string downloadUrl;
    std::string localSavedPath;
    std::string name;
    int tag;
    FileManager::FileType fileType;
    DownloadInfo() {
        clear();
    }
    ~DownloadInfo() {
        clear();
    }
    void clear() {
        data.clear();
        bSave = false;
        downloadUrl = "";
        localSavedPath = "";
        name = "";
        fileType = FileManager::FileType::Image;
    }
};

class SMDownloaderListener {
public:
    virtual void onDownloadFinished(bool Success, int tag, Intent * param) = 0;
    virtual void onDownloadProgress(int tag, const int idx, const int total, float progress) = 0;
};

class SMDownloader : public WorkThreadProtocol {
public:
    static SMDownloader * getInstance() {
        static SMDownloader * instance = nullptr;
        if (instance==nullptr) {
            instance = new (std::nothrow)SMDownloader();
        }
        
        return instance;
    }
    
    void addDownloadTask(std::string name, std::string url, FileManager::FileType fileType, bool bDiskCache, int tag);
    void setDownloadListener(SMDownloaderListener * l) { _listener = l;}
    
protected:
    SMDownloader();
    virtual ~SMDownloader();
    
    // work thread
    virtual void onJobFinish(const int tag, const bool success, Intent* intent) override;
    virtual void onJobProgress(const int tag, const int idx, const int total, const float progress, std::string desc) override;

    
private:
    SMDownloaderListener * _listener;
};

class SMDownloadFunction : public ToDoFunction
{
public:
    SMDownloadFunction(int tag, DownloadInfo * info) {
        _info = new (std::nothrow)DownloadInfo();
        _info->downloadUrl = info->downloadUrl;
        _info->bSave = info->bSave;
        _info->name = info->name;
        _info->data.clear();
        _info->fileType = info->fileType;
        _info->tag = tag;
        _tag = tag;
    }
    
    ~SMDownloadFunction() {
        // 죽을 때 release
        CC_SAFE_RELEASE(_info);
    }
    
protected:
    // 작업전에
    virtual bool onBeforeDoJob() override;
    
    // 작업중에
    virtual bool onJobInBackground() override;
    
    
    // 작업 끝나고 다운로드 된거 뱉어
    virtual Intent* onAfterDoneJob() override;

    void initCurlHandleProc(CURL* handle, bool existBody=false);
    size_t writeDataProc(unsigned char* buffer, size_t size, size_t count);
    static size_t outputDataCallbackProc(void* buffer, size_t size, size_t count, void* callerPtr);

    static int progressCallbackProc(void *callerPtr,   curl_off_t dltotal,   curl_off_t dlnow,   curl_off_t ultotal,   curl_off_t ulnow);
    
private:
    DownloadInfo * _info;
    int _tag;
    bool _bSuccess;
};




#endif /* SMDownloader_h */
