//
//  DownloadTask.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 16..
//
//

#ifndef DownloadTask_h
#define DownloadTask_h

#include "DownloadProtocol.h"
#include "MemoryCache.h"
#include "ImageCache.h"
#include "ImageDownloader.h"
#include <memory>
#include <string>
#include <mutex>
#include <platform/CCImage.h>

#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
#include <external/curl/include/ios/curl/curl.h>
#include <CoreGraphics/CGImage.h>
#elif CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
#include <external/curl/include/android/curl/curl.h>
#elif CC_TARGET_PLATFORM == CC_PLATFORM_MAC
#include <external/curl/include/mac/curl/curl.h>
#include <CoreGraphics/CGImage.h>
#elif CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
#include <external/curl/include/win32/curl/curl.h>
#endif


class DownloadTask : public std::enable_shared_from_this<DownloadTask> {
public:
    enum MediaType {
        NETWORK,
        RESOURCE,
        FILE,
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
        IOSIMAGE,
        IOSTHUMBNAIL
#else
        ANDROIDIMAGE,
        ANDROIDTHUMB,
#endif
    };
    
    static std::shared_ptr<DownloadTask> createTaskForTarget(ImageDownloader* downloader, DownloadProtocol* target);
    static std::string makeCacheKey(MediaType type, const std::string& requestPath, DownloadConfig* config, std::string* keyPath);
    
    
    void init(MediaType type, const std::string& requestPath, DownloadConfig* config);
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    void setPHAssetObject(void* phAssetObject) { _phAssetObject = phAssetObject; }
    std::string getPHAssetIdentifier();
#endif

    inline void interrupt() {_running = false;}
    inline bool isRunning() {return _running;}
    inline bool isTargetAlive() {return !_downloadTargetAlive.expired();}
    
    int getTag() const {return _tag;}
    void setTag(const int tag) {_tag = tag;}
    
    MediaType getMediaType() {return _type;}
    DownloadProtocol* getTarget() {return _target;}
    std::string& getCacheKey() {return _cacheKey;}
    std::string& getRequestPath() {return _requestPath;}
    std::string& getKeyPath() {return _keyPath;}
    
    std::shared_ptr<MemoryCacheEntry> getMemoryCacheEntry() {return _cacheEntry;}
    void setMemoryCacheEntry(std::shared_ptr<MemoryCacheEntry>* cacheEntry) {_cacheEntry = *cacheEntry;}
    
    std::shared_ptr<ImageCacheEntry> getImageCacheEntry() {return _imageEntry;}
    void setImageCacheEntry(std::shared_ptr<ImageCacheEntry>* imageEntry) {_imageEntry = *imageEntry;}
    
    void setDecodedImage(std::shared_ptr<ImageCacheEntry> entry) {_imageEntry = entry;}
    
    DownloadConfig& getConfig() {return _config;}
    
    ImageDownloader* getDownloader() {return _downloader;}
    
public:
    // network
    void procDownloadThread();
    
    // reosource
    void procLoadFromResourceThread();
    
    // file
    void procLoadFromFileThread();
    
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    // deprecated
//    void procLoadFromNSURLThread();
//    void procLoadFromNSURLThumbnailThread();
    void procLoadFromLocalIdentifierThread();
    void procLoadFromLocalIdentifierThumbnailThread();
    
    CGImageRef _imageRef;
#else
    void procLoadFromAndroidLocalFileThread();
    void procLoadFromAndroidThumbnailThread();
#endif
    
    void procDecodeThread();
    
    cocos2d::Image* getResampleImage(cocos2d::Image** srcImage);
    
public:
    DownloadTask();
    virtual ~DownloadTask();
    
public:
//    static void initAssetLibrary();
//    static void releaseAssetLibrary();
    
private:
    void initCurlHandleProc(CURL* handle, bool forContent=false);
    size_t writeDataProc(unsigned char* buffer, size_t size, size_t count);
    static size_t outputDataCallbackProc(void* buffer, size_t size, size_t count, void* userdata);
    
private:
    bool _running;
    int _tag;
    MediaType _type;
    DownloadConfig _config;
    std::shared_ptr<MemoryCacheEntry> _cacheEntry;
    std::string _cacheKey;
    std::string _requestPath;
    std::string _keyPath;
    std::weak_ptr<bool> _downloadTargetAlive;
    DownloadProtocol* _target;
    std::shared_ptr<ImageCacheEntry> _imageEntry;
    ImageDownloader* _downloader;
    int _taskId;

    void* _phAssetObject;
    void* _uiImageObject;

    uint8_t* _assetData;
    int _assetDataWidth;
    int _assetDataHeight;
};

class FileCacheWriteTask : public std::enable_shared_from_this<FileCacheWriteTask>
{
public:
    static std::shared_ptr<FileCacheWriteTask> createTaskForCache(std::string& cacheKey, std::shared_ptr<MemoryCacheEntry> cacheEntry);
    
    void procFileCacheWriteThread();
    
private:
    std::shared_ptr<MemoryCacheEntry> _cacheEntry;
    std::string _cacheKey;
};


#endif /* DownloadTask_h */
