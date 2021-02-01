//
//  ImageDownloader.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 16..
//
//

#ifndef ImageDownloader_h
#define ImageDownloader_h

#include "DownloadProtocol.h"
#include <string>
#include <memory>
#include <mutex>

#define DOWNLOAD_FROM_NETWORK  (0)
#define DOWNLOAD_FROM_ASSERTS  (1)
#define DOWNLOAD_FROM_FILE   (2)


struct DownloadConfig {
public:
    enum CachePolycy {
        
        // default -> network(image, memory, disk) : ALL_CACHE, library(image, mem) : NO_DISK, file(image, mem) : NO_DISK
        DEFAULT,
        
        NO_CACHE,
        
        ALL_CACHE, // using all cache : image, memory, disk

        IMAGE_ONLY, // image cache
        
        MEMORY_ONLY, // memory cache
        
        DISK_ONLY,  // disk cache
        
        NO_IMAGE,
        
        NO_MEMORY,
        
        NO_DISK
    };


	enum ResampleMethod {
		ResampleMethod_NEAREST,
		ResampleMethod_LINEAR,
		ResampleMethod_CUBIC,
		ResampleMethod_LANCZOS
	};
    
    enum ResamplePolicy {
        NONE,
        
        EXACT_FIT, // fit by size(width, height) 비율 상관 없음
        
        EXACT_CROP, // fit by small length and crop...
        
        AREA, // view보다 작은 크기로 fit
        
        LONGSIDE, // fit by big length to param1
        
        SCALE, // scale from param1 value
    };


public:
    void setCachePolicy(const CachePolycy cachePolicy);
    void setResamplePolicy(const ResamplePolicy resamplePolicy, const float param1=0.0, const float param2=0.0);
    void setResampleMethod(const ResampleMethod resampleMethod);
    void setRotation(const int degress);
    void setPhysicsBodyEnable(bool enable, const float epsilon=2.0, const float threshold=0.05);
    void setCacheOnly() {_cacheOnly = true;}
    void setSmallThumbnail() {_smallThumbnail = true;}
    DownloadConfig(const CachePolycy cachePolicy=CachePolycy::DEFAULT, const bool cacheOnly=false);
    
    bool isEnableMemoryCache() {return _enableMemoryCache;}
    bool isEnableImageCache() {return _enableImageCache;}
    bool isEnableDisckCache() {return _enableDiskCache;}
    bool isEnablePhysicsBody() {return _enablePhysicsBody;}
    bool isCacheOnly() {return _cacheOnly;}
    bool isSmallThumbnail() {return _smallThumbnail;}
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS    
    void setMaxSideLength(int maxSideLength);
#endif    
    
private:
    CachePolycy _cachePolicy;
    ResamplePolicy _resamplePolicy;
    ResampleMethod _resampleMethod;
    
    float _resParam1;
    float _resParam2;
    int _reqDegress;
    
    bool _resampleShrinkOnly;
    
    bool _cacheOnly;
    
    bool _enableMemoryCache;
    bool _enableImageCache;
    bool _enableDiskCache;
    bool _enablePhysicsBody;
    
    bool _smallThumbnail;
    
    float _physicsBodyEpsilon;
    float _physicsBodyThreshold;
    
    friend class DownloadTask;
    friend class ImageDownloader;
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS    
    int _maxSideLength;
#endif    
};

class MemoryLRUCache;
class MemoryCacheEntry;

class ImageLRUCache;
class ImageCacheEntry;

class DownloadTask;

class ImageDownloader {
public:
    static const DownloadConfig DC_DEFAULT;
    static const DownloadConfig DC_NO_CACHE;
    static const DownloadConfig DC_NO_DISK;
    static const DownloadConfig DC_NO_IMAGE;
    static const DownloadConfig DC_CACHE_ONLY;
    static const DownloadConfig DC_CACHE_ONLY_NO_IMAGE;
    static const DownloadConfig DC_CACHE_ONLY_NO_DISK;
    static const DownloadConfig DC_CACHE_ONLY_DISK_ONLY;
    static const DownloadConfig DC_IMAGE_ONLY;
    
public:
    enum class State {
        DOWNLOAD_STARTED,
        DOWNLOAD_SUCCESS,
        DOWNLOAD_FAILED,
        
        DECODE_STARTED,
        DECODE_SUCCESS,
        DECODE_FAILED,
        
        IMAGE_CACHE_DIRECT,
    };
    
    static ImageDownloader& getInstance();
    static ImageDownloader& getInstanceForFitting(); // ???? what is this???
    
    void loadImageFromNetwork(DownloadProtocol* target, const std::string& requestUrl, const int tag=0, DownloadConfig* config=nullptr);
    void loadImageFromResource(DownloadProtocol* target, const std::string& requestUrl, const int tag=0, DownloadConfig* config=nullptr);
    void loadImageFromFile(DownloadProtocol* target, const std::string& requestUrl, const int tag=0, DownloadConfig* config=nullptr);
    
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
    // deprecated
//    void loadImageFromNSURL(DownloadProtocol* target, const std::string& requestUrl, const int tag=0, DownloadConfig* config=nullptr);
//    void loadThumbnailFromNSURL(DownloadProtocol* target, const std::string& requestUrl, const int tag=0, DownloadConfig* config=nullptr);
    void loadImageFromLocalIdentifier(DownloadProtocol* target, const std::string& requestID, const int tag=0, DownloadConfig* config=nullptr);
    void loadThumbnailFromLocalIdentifier(DownloadProtocol* target, const std::string& requestID, const int tag=0, DownloadConfig* config=nullptr);

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    void loadImagePHAssetThumbnail(DownloadProtocol* target, void* phAssetObject, const int tag = 0, DownloadConfig* config = nullptr);
    void loadImagePHAsset(DownloadProtocol* target, void* phAssetObject, const int tag = 0, DownloadConfig* config = nullptr);
#endif

#else
    void loadImageFromAndroidLocalFile(DownloadProtocol* target, const std::string& requestUrl, const int tag=0, DownloadConfig* config=nullptr);
    void loadImageFromAndroidThumbnail(DownloadProtocol* target, const std::string& requestUrl, const int tag=0, DownloadConfig* config=nullptr);
#endif
    
    void cancelImageDownload(DownloadProtocol* target);

    static cocos2d::PhysicsBody* polygonInfoToPhysicsBody(const cocos2d::PolygonInfo* info, const cocos2d::Size& imageSize);
    
    bool isCachedForNetwork(const std::string& requestPath, DownloadConfig* config) const;
    
    void clearCache();
    
    bool saveToFileCache(const std::string& requestPath, uint8_t* data, ssize_t length, DownloadConfig* config=nullptr);
    
protected:
    MemoryLRUCache* getMemCache() {return _memCache;}
    ImageLRUCache* getImageCache() {return _imageCache;}
    
    void queueDownloadTask(DownloadProtocol* target, const std::shared_ptr<DownloadTask>& task);
    
    void addDownloadTask(const std::function<void()>& task);
    void addDecodeTask(const std::function<void()>& task);
    
    void handleState(std::shared_ptr<DownloadTask> task, State state, std::shared_ptr<ImageCacheEntry>* imageEntry=nullptr);
    
    void writeToFileCache(std::string& cacheKey, std::shared_ptr<MemoryCacheEntry> cacheEntry);
    
    ImageDownloader(const int memCacheSize, const int imageCacheSize, const int downloadPoolSize, const int decodePoolSize);
    
    ~ImageDownloader();
    
    void init();
    
    cocos2d::PolygonInfo* makePolygonInfo(cocos2d::Image* image, const float epsilon, const float threshold);
    
private:
    class ThreadPool;
    
    static const int MEM_CACHE_SIZE;
    static const int IMAGE_CACHE_SIZE;

    static const int CORE_POOL_SIZE;
    static const int MAXIMUM_POOL_SIZE;
    
    ThreadPool* _downloadThreadPool;
    ThreadPool* _decodeThreadPool;
    ThreadPool* _fileCacheWriteThreadPool;
//    ThreadPool* _decompressThreadPool;
    
    MemoryLRUCache* _memCache;
    ImageLRUCache* _imageCache;
    
    std::mutex _mutex_download;
    std::mutex _mutex_decode;
    std::mutex _mutex_file;
    std::mutex _mutex_physics;
    
    int _memCacheSize;
    int _imageCacheSize;
    
    int _downloadPoolSize;
    int _decodePoolSize;
    
    friend class DownloadTask;
};

#endif /* ImageDownloader_h */
