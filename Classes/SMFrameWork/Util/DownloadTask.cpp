//
//  DownloadTask.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 16..
//
//

#include <platform/CCPlatformConfig.h>
#include "ImageDownloader.h"
#include "DownloadTask.h"
#include "md5.h"
#include "FileManager.h"
#include "../Base/ImageAutoPolygon.h"

#include <cocos2d.h>

#include <chrono>
#include <algorithm>

#include "use_opencv.h"

#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID

#include <platform/android/jni/JniHelper.h>
#include <jni.h>
#include <android/log.h>
#define  LOG_TAG    "main"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)

#elif CC_TARGET_PLATFORM == CC_PLATFORM_MAC
#include <opencv2/imgproc.hpp>
#elif CC_PLATFORM_WIN32
#endif


#define TIMEOUT_IN_SECOND   (10L)
#define LOW_SPEED_LIMIT     (1L)
#define LOW_SPEED_TIME      (5L)
#define MAX_REDIRS          (2L)
#define MAX_WIDTH       (160)

#define HTTP_SUCCESS 200

#define __CHECK_THREAD_INTERRUPTED__  { std::this_thread::sleep_for(std::chrono::milliseconds(1)); if (!_running) break; }

std::shared_ptr<DownloadTask> DownloadTask::createTaskForTarget(ImageDownloader *downloader, DownloadProtocol *target)
{
    std::shared_ptr<DownloadTask> task = std::make_shared<DownloadTask>();
    
    task->_target = target;
    task->_downloadTargetAlive = target->_downloadTargetAlive;
    task->_downloader = downloader;
    
    return task;
}

static int __task_count__ = 0;

DownloadTask::DownloadTask() :
_running(true)
, _cacheEntry(nullptr)
{
    _taskId = 0x100 + __task_count__++;
//    _cacheEntry = MemoryCacheEntry::createEntry();
    
}

DownloadTask::~DownloadTask()
{
    --__task_count__;
    
    _cacheEntry.reset();
    _imageEntry.reset();
}

std::string DownloadTask::makeCacheKey(DownloadTask::MediaType type, const std::string &requestPath, DownloadConfig *config, std::string *keyPath)
{
    DownloadConfig newConfig = *config;
    std::string key;
    
    switch (type) {
        case MediaType::NETWORK :
        {
//            key = requestPath.substr(0, requestPath.find("?"));
            key = requestPath;
        }
            break;
        case MediaType::RESOURCE :
        {
            key = std::string("@RES:").append(requestPath);
        }
            break;
        case MediaType::FILE :
        {
            key = std::string("@FILE:").append(requestPath);
        }
            break;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
        case MediaType::IOSIMAGE :
        {
//            key = std::string("@NSURL:").append(requestPath);
            key = std::string("@PHAS:").append(requestPath);
        }
            break;
        case MediaType::IOSTHUMBNAIL :
        {
//            if (newConfig.isSmallThumbnail()) {
//                // smaill thumbnail
//                key = std::string("@STHUMB:").append(requestPath);
//            } else {
//                // thumbnail
//                key = std::string("@THUMB:").append(requestPath);
//            }
            key = std::string("@PHTB:").append(requestPath);
        }
            break;
#else
        case MediaType::ANDROIDIMAGE:
        {
            key = std::string("@ANDROIDIMAGE").append(requestPath);
        }
            break;
        case MediaType::ANDROIDTHUMB:
        {
            key = std::string("@ANDROIDTHUMB").append(requestPath);
        }
            break;
#endif
    }
    
    std::string resampleKey;
    switch (newConfig._resamplePolicy) {
        case DownloadConfig::ResamplePolicy::EXACT_FIT:
        {
            resampleKey = cocos2d::StringUtils::format("_@EXACT_FIT_%dx%d_", (int)newConfig._resParam1, (int)newConfig._resParam2);
        }
            break;
        case DownloadConfig::ResamplePolicy::EXACT_CROP:
        {
            resampleKey = cocos2d::StringUtils::format("_@EXACT_CROP_%dx%d_", (int)newConfig._resParam1, (int)newConfig._resParam2);
        }
            break;
        case DownloadConfig::ResamplePolicy::AREA:
        {
            resampleKey = cocos2d::StringUtils::format("_@AREA_%dx%d_", (int)newConfig._resParam1, (int)newConfig._resParam2);
        }
            break;
        case DownloadConfig::ResamplePolicy::LONGSIDE:
        {
            resampleKey = cocos2d::StringUtils::format("_@LONGSIDE_%dx%d_", (int)newConfig._resParam1, (int)newConfig._resParam2);
        }
            break;
        case DownloadConfig::ResamplePolicy::SCALE:
        {
            resampleKey = cocos2d::StringUtils::format("_@SCALE_%f", newConfig._resParam1);
        }
            break;
        default:
        {
            resampleKey = "";
        }
            break;
    }
    
    key.append(resampleKey);
    
    if (keyPath) {
        *keyPath = key;
    }
    
    return md5(key);
}

void DownloadTask::init(DownloadTask::MediaType type, const std::string &requestPath, DownloadConfig *config)
{
    _type = type;
    _requestPath = requestPath;
    
    if (config==nullptr) {
        _config.setCachePolicy(DownloadConfig::CachePolycy::DEFAULT);
    } else {
        _config = *config;
    }
    
    _cacheKey = makeCacheKey(type, requestPath, &_config, &_keyPath);
    
    switch (_config._cachePolicy) {
        case DownloadConfig::CachePolycy::DEFAULT:
        {
            
            _config._enableImageCache= true;
            _config._enableMemoryCache = true;
            _config._enableDiskCache = type==MediaType::NETWORK;
        }
            break;
        case DownloadConfig::CachePolycy::NO_CACHE:
        {
            
            _config._enableImageCache = false;
            _config._enableMemoryCache = false;
            _config._enableDiskCache = false;
        }
            break;
        case DownloadConfig::CachePolycy::ALL_CACHE:
        {
            
            _config._enableImageCache = true;
            _config._enableMemoryCache = true;
            _config._enableDiskCache = true;
        }
            break;
        case DownloadConfig::CachePolycy::IMAGE_ONLY:
        {
            
            _config._enableImageCache = true;
            _config._enableMemoryCache = false;
            _config._enableDiskCache = false;
        }
            break;
        case DownloadConfig::CachePolycy::MEMORY_ONLY:
        {
            
            _config._enableImageCache = false;
            _config._enableMemoryCache = true;
            _config._enableDiskCache = false;
        }
            break;
        case DownloadConfig::CachePolycy::DISK_ONLY:
        {
            
            _config._enableImageCache = false;
            _config._enableMemoryCache = false;
            _config._enableDiskCache = true;
        }
            break;
        case DownloadConfig::CachePolycy::NO_IMAGE:
        {
            
            _config._enableImageCache = false;
            _config._enableMemoryCache = true;
            _config._enableDiskCache = true;
        }
            break;
        case DownloadConfig::CachePolycy::NO_MEMORY:
        {
            
            _config._enableImageCache = true;
            _config._enableMemoryCache = false;
            _config._enableDiskCache = true;
        }
            break;
        case DownloadConfig::CachePolycy::NO_DISK:
        {
            
            _config._enableImageCache = true;
            _config._enableMemoryCache = true;
            _config._enableDiskCache = false;
        }
            break;
    }
    
    if (_config._enableDiskCache && type!=MediaType::NETWORK) {
        
        _config._enableDiskCache = false;
    }
}

size_t DownloadTask::writeDataProc(unsigned char *buffer, size_t size, size_t count)
{
    size_t readSize = size * count;
    
    if (readSize>0) {
        _cacheEntry->appendData((uint8_t*)buffer, readSize);
    }
    
    return readSize;
}

size_t DownloadTask::outputDataCallbackProc(void *buffer, size_t size, size_t count, void *userdata)
{
    DownloadTask * task = static_cast<DownloadTask*>(userdata);
    
    if (!task->isRunning()) {
        
        return -1;
    }
    
    return task->writeDataProc((unsigned char*)buffer, size, count);
}

void DownloadTask::initCurlHandleProc(CURL *handle, bool forContent)
{
    // CURL Handler...
    // CURL settting
    // thread callback func
    
    // url setting
    curl_easy_setopt(handle, CURLOPT_URL, _requestPath.c_str());
    
    // time out signal setting
    curl_easy_setopt(handle, CURLOPT_NOSIGNAL, 1);
    
    // write func settng
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, outputDataCallbackProc);
    
    // write data setting
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, this);
    
    // progress setting
    curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 1);
    
    // fail on error settting
    curl_easy_setopt(handle, CURLOPT_FAILONERROR, 1);
    
    if (!forContent) {
        // only header
        curl_easy_setopt(handle, CURLOPT_HEADER, 1);
        // no body
        curl_easy_setopt(handle, CURLOPT_NOBODY, 1);
    }
    
    curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT, TIMEOUT_IN_SECOND);
    curl_easy_setopt(handle, CURLOPT_LOW_SPEED_LIMIT, LOW_SPEED_LIMIT);
    curl_easy_setopt(handle, CURLOPT_LOW_SPEED_TIME, LOW_SPEED_TIME);
    
    if (MAX_REDIRS) {
        curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_setopt(handle, CURLOPT_MAXREDIRS, MAX_REDIRS);
    }

    // need to test.
//    size_t pos = _requestPath.find("https://");
//    if (pos!=-1) {
//        // must do ssl
        curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0L);
//    } else {
//        // must do ssl
//        curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 1L);
//        curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 1L);
//    }
    
}

// download thread func
void DownloadTask::procDownloadThread()
{
    
    CURL * curl = nullptr;
    
    do {
        __CHECK_THREAD_INTERRUPTED__;
        
        
        _cacheEntry.reset();
        
        if (_config.isEnableMemoryCache()) {
            
            auto cacheEntry = _downloader->getMemCache()->get(_cacheKey);
            if (cacheEntry && (*cacheEntry)->size()>0) {
                
                _cacheEntry = *cacheEntry;
                _downloader->handleState(shared_from_this(), ImageDownloader::State::DOWNLOAD_SUCCESS);
                return;
            }
        }
        
        __CHECK_THREAD_INTERRUPTED__;
        
        if (_config.isEnableDisckCache()) {
            
            int error;
            auto data = FileManager::getInstance()->loadFromFile(FileManager::FileType::Image, _cacheKey, &error);
            
            __CHECK_THREAD_INTERRUPTED__;
            
            if (error==FileManager::SUCCESS && data.getSize()>0) {
                
                
                _cacheEntry = MemoryCacheEntry::createEntry(data.getBytes(), data.getSize());
                data.fastSet(nullptr, 0);
                
                if (_config.isEnableMemoryCache()) {
                    
                    _downloader->getMemCache()->insert(_cacheKey, _cacheEntry);
                    _downloader->getMemCache()->printfDownloadStat("MEM CACHE"); 
                }
                
                __CHECK_THREAD_INTERRUPTED__;
                
                
                _downloader->handleState(shared_from_this(), ImageDownloader::State::DOWNLOAD_SUCCESS);
                return;
            }
        }
        
        
        curl = curl_easy_init();
        if (curl==nullptr) {
            CCLOG("Failed to CURL Init()");
            break;
        }
        
        __CHECK_THREAD_INTERRUPTED__;
        
        // curl option setting
        initCurlHandleProc(curl, true);
        
        __CHECK_THREAD_INTERRUPTED__;
        
        _cacheEntry = MemoryCacheEntry::createEntry();
        
        CURLcode retCode = curl_easy_perform(curl);
        
        // curl error code check
        if (retCode != CURLE_OK) {
            CCLOG("Failed to curl download!!!!");
            break;
        }
        
        long responseCode = 0;
        
        retCode = curl_easy_getinfo(curl, CURLINFO_HTTP_CODE, &responseCode);
        
        if (retCode != CURLE_OK || responseCode!=HTTP_SUCCESS) {
            CCLOG("Failed to COM error for curl download");
            break;
        }
        
        
        
        _cacheEntry->shrinkToFit();
        
        if (_config.isEnableMemoryCache()) {
            
            _downloader->getMemCache()->insert(_cacheKey, _cacheEntry);
            _downloader->getMemCache()->printfDownloadStat("MEM CACHE");    
        }
        
        
        if (curl) {
            curl_easy_cleanup(curl);
        }
        
        
        _downloader->handleState(shared_from_this(), ImageDownloader::State::DOWNLOAD_SUCCESS);
        
        if (_config.isEnableDisckCache()) {
            
            _downloader->writeToFileCache(_cacheKey, _cacheEntry);
        }
        
        return;
    } while (0);
    
    
    _downloader->handleState(shared_from_this(), ImageDownloader::State::DOWNLOAD_FAILED);
    _cacheEntry.reset();
    if (curl) {
        curl_easy_cleanup(curl);
    }
}

// load from resource thread func
void DownloadTask::procLoadFromResourceThread()
{
    
    do {
        __CHECK_THREAD_INTERRUPTED__;
        
        _cacheEntry.reset();
        
        if (_config.isEnableMemoryCache()) {
            
            auto cacheEntry = _downloader->getMemCache()->get(_cacheKey);
            if (cacheEntry && (*cacheEntry)->size()>0) {
                
                _cacheEntry = *cacheEntry;
                _downloader->handleState(shared_from_this(), ImageDownloader::State::DOWNLOAD_SUCCESS);
                return;
            }
        }
        
        __CHECK_THREAD_INTERRUPTED__;
        
        if (_config.isEnableDisckCache()) {
            
            int error;
            auto data = FileManager::getInstance()->loadFromFile(FileManager::FileType::Image, _cacheKey, &error);
            
            __CHECK_THREAD_INTERRUPTED__;
            
            if (error==FileManager::SUCCESS && data.getSize()>0) {
                
                _cacheEntry = MemoryCacheEntry::createEntry(data.getBytes(), data.getSize());
                data.fastSet(nullptr, 0);
                
                if (_config.isEnableMemoryCache()) {
                    
                    _downloader->getMemCache()->insert(_cacheKey, _cacheEntry);
                    _downloader->getMemCache()->printfDownloadStat("MEM CACHE");    
                }
                
                __CHECK_THREAD_INTERRUPTED__;
             
                
                _downloader->handleState(shared_from_this(), ImageDownloader::State::DOWNLOAD_SUCCESS);
                return;
            }
        }
        
        
        
        auto fileUtils = cocos2d::FileUtils::getInstance();
        auto filePath = fileUtils->fullPathForFilename(_requestPath);
        auto data = fileUtils->getDataFromFile(filePath);
        
        if (data.getBytes()==nullptr || data.getSize()==0) {
            CCLOG("Failed to resource file loading!");
            break;
        }
        
        
        _cacheEntry = MemoryCacheEntry::createEntry(data.getBytes(), data.getSize());
        data.fastSet(nullptr, 0);
        
        _downloader->handleState(shared_from_this(), ImageDownloader::State::DOWNLOAD_SUCCESS);
        
        if (_config.isEnableMemoryCache()) {
            
            _downloader->getMemCache()->insert(_cacheKey, _cacheEntry);
            _downloader->getMemCache()->printfDownloadStat("MEM CACHE");
        }
        
        __CHECK_THREAD_INTERRUPTED__;
        
        if (_config.isEnableDisckCache()) {
            
            _downloader->writeToFileCache(_cacheKey, _cacheEntry);
        }
        
        return;
    } while (0);
    
    
    _cacheEntry.reset();
    _downloader->handleState(shared_from_this(), ImageDownloader::State::DOWNLOAD_FAILED);
}

// load from file thread func
void DownloadTask::procLoadFromFileThread()
{
    
    do {
        __CHECK_THREAD_INTERRUPTED__;
        
        
        auto cacheEntry = _downloader->getMemCache()->get(_cacheKey);
        
        if (cacheEntry && (*cacheEntry)->size()>0) {
            
            _cacheEntry = *cacheEntry;
            _downloader->handleState(shared_from_this(), ImageDownloader::State::DOWNLOAD_SUCCESS);
            return;
        } else {
            
            _cacheEntry.reset();
        }
        
        __CHECK_THREAD_INTERRUPTED__;
        
        
        auto fileUtil = cocos2d::FileUtils::getInstance();
        auto filePath = fileUtil->getWritablePath() + _requestPath;
        auto data = fileUtil->getDataFromFile(filePath);

        if (data.getBytes()==nullptr || data.getSize()==0) {
            CCLOG("[[[[[ Failed to load from file 1");
            break;
        }

        _cacheEntry = MemoryCacheEntry::createEntry(data.getBytes(), data.getSize());
        data.fastSet(nullptr, 0);
        
        _downloader->getMemCache()->insert(_cacheKey, _cacheEntry);
        _downloader->getMemCache()->printfDownloadStat("MEM CACHE");
        
        _downloader->handleState(shared_from_this(), ImageDownloader::State::DOWNLOAD_SUCCESS);
        return;
        
    } while (0);
    
    
    _cacheEntry.reset();
    _downloader->handleState(shared_from_this(), ImageDownloader::State::DOWNLOAD_FAILED);
}
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
void DownloadTask::procLoadFromAndroidLocalFileThread()
{
    
    do {
        __CHECK_THREAD_INTERRUPTED__;
        _cacheEntry.reset();
        /*
        
        auto cacheEntry = _downloader->getMemCache()->get(_cacheKey);

        if (cacheEntry && (*cacheEntry)->size()>0) {
            
            _cacheEntry = *cacheEntry;
            _downloader->handleState(shared_from_this(), ImageDownloader::State::DOWNLOAD_SUCCESS);
            return;
        }

        __CHECK_THREAD_INTERRUPTED__;

        
        auto fileUtil = cocos2d::FileUtils::getInstance();
        auto filePath = _requestPath;
        auto data = fileUtil->getDataFromFile(filePath);

        if (data.getBytes()==nullptr || data.getSize()==0) {
            CCLOG("[[[[[ Failed to load from file 1");
            break;
        }

        _cacheEntry = MemoryCacheEntry::createEntry(data.getBytes(), data.getSize());
        data.fastSet(nullptr, 0);

        _downloader->getMemCache()->insert(_cacheKey, _cacheEntry);
        _downloader->getMemCache()->printfDownloadStat("MEM CACHE");
        
        _downloader->handleState(shared_from_this(), ImageDownloader::State::DOWNLOAD_SUCCESS);
        return;
        */
        LOGD("[[[[[[[[[[[[ procLoadFromAndroidLocalFileThread !!!!! ");
        auto srcImage = new cocos2d::Image();

        cocos2d::JniMethodInfo getPhotoImage;
        if (!cocos2d::JniHelper::getStaticMethodInfo(getPhotoImage, "org/cocos2dx/cpp/AppActivity", "getPhotoImage", "(Ljava/lang/String;)[B")) {
            LOGD("[[[[[ failed to get static method.... getPhotothumbnail....");
            break;
        }

        jstring imgUrlArg = getPhotoImage.env->NewStringUTF(_requestPath.c_str());
        jbyteArray imageBytes = (jbyteArray)getPhotoImage.env->CallStaticObjectMethod(getPhotoImage.classID, getPhotoImage.methodID, imgUrlArg);
        if (imageBytes==nullptr) {
            LOGD("[[[[[ getting image is null 1");
            break;
        }

        int len  = getPhotoImage.env->GetArrayLength(imageBytes );
        unsigned char * bytes = (unsigned char*)malloc(sizeof(unsigned char)*len);

        jbyte* imgData = getPhotoImage.env->GetByteArrayElements(imageBytes, NULL);
        if (imgData == NULL) {
            LOGD("[[[[[ getting image is null 2");
            break;
        }

        memcpy(bytes, imgData, len);
        getPhotoImage.env->ReleaseByteArrayElements(imageBytes, imgData, JNI_ABORT);

        //getPhotoImage.env->GetByteArrayRegion(imageBytes, 0, len, (jbyte*)bytes);



        bool existImage = srcImage->initWithImageData(bytes, len);
        free(bytes);

        getPhotoImage.env->DeleteLocalRef(imgUrlArg);
        getPhotoImage.env->DeleteLocalRef(imageBytes);
        getPhotoImage.env->DeleteLocalRef(getPhotoImage.classID);



        if (existImage) {
            LOGD("[[[[[[[[[[[[ SUCCESS load image from ANDROID BITMAP Factory !!!!! ");

            _imageEntry = ImageCacheEntry::createEntry(srcImage , nullptr);
            _downloader->handleState(shared_from_this(), ImageDownloader::State::DECODE_SUCCESS);
            return;
        }
        CC_SAFE_DELETE(srcImage);

    } while (0);

    
    _cacheEntry.reset();
    _downloader->handleState(shared_from_this(), ImageDownloader::State::DOWNLOAD_FAILED);
}

void DownloadTask::procLoadFromAndroidThumbnailThread()
{
    _imageEntry.reset();
    
    do {
        __CHECK_THREAD_INTERRUPTED__;

        _cacheEntry.reset();


        __CHECK_THREAD_INTERRUPTED__;

        auto thumbImage = new cocos2d::Image();

        cocos2d::JniMethodInfo getPhotoThumbnail;
        if (!cocos2d::JniHelper::getStaticMethodInfo(getPhotoThumbnail, "org/cocos2dx/cpp/AppActivity", "getPhotoThumbnail", "(Ljava/lang/String;)[B")) {
            LOGD("[[[[[ failed to get static method.... getPhotothumbnail....");
            break;
        }

        jstring imgUrlArg = getPhotoThumbnail.env->NewStringUTF(_requestPath.c_str());
        jbyteArray thumbBytes = (jbyteArray)getPhotoThumbnail.env->CallStaticObjectMethod(getPhotoThumbnail.classID, getPhotoThumbnail.methodID, imgUrlArg);
        if (thumbBytes==nullptr) {
            LOGD("[[[[[ getting thumbnail is null 1");
            break;
        }

        int len  = getPhotoThumbnail.env->GetArrayLength(thumbBytes );
        unsigned char * bytes = (unsigned char*)malloc(sizeof(unsigned char)*len);

        jbyte* thumbData = getPhotoThumbnail.env->GetByteArrayElements(thumbBytes, NULL);
        if (thumbData == NULL) {
            LOGD("[[[[[ getting thumbnail is null 2");
            break;
        }

        memcpy(bytes, thumbData, len);
        getPhotoThumbnail.env->ReleaseByteArrayElements(thumbBytes, thumbData, JNI_ABORT);


        //getPhotoThumbnail.env->GetByteArrayRegion(thumbBytes, 0, len, (jbyte*)bytes);



        bool existThumb = thumbImage->initWithImageData(bytes, len);
        free(bytes);

        getPhotoThumbnail.env->DeleteLocalRef(imgUrlArg);
        getPhotoThumbnail.env->DeleteLocalRef(thumbBytes);
        getPhotoThumbnail.env->DeleteLocalRef(getPhotoThumbnail.classID);



        if (existThumb) {
            //LOGD("[[[[[[[[[[[[ SUCCESS load image from ANDROID BITMAP Factory !!!!! ");

            _imageEntry = ImageCacheEntry::createEntry(thumbImage, nullptr);
            _downloader->handleState(shared_from_this(), ImageDownloader::State::DECODE_SUCCESS);
            return;
        }

        CC_SAFE_DELETE(thumbImage);


/*
        auto srcImage = new cocos2d::Image();
        bool bExistCache = false;
        

        

        if (bExistCache==false) {
            
            auto fileUtil = cocos2d::FileUtils::getInstance();
            auto filePath = _requestPath;
            auto data = fileUtil->getDataFromFile(filePath);
            if (data.getBytes()==nullptr || data.getSize()==0) {
                LOGD("[[[[[ Failed to load from file 1");
                return;
            }

            auto srcImage = new cocos2d::Image();
            if (!srcImage->initWithImageData(data.getBytes(), data.getSize())) {
                CC_SAFE_RELEASE_NULL(srcImage);
                LOGD("Failed to Decoding image");
                break;
            }

            int type = CV_8UC4;
            cocos2d::Texture2D::PixelFormat format = srcImage->getRenderFormat();
            switch (format) {
                case cocos2d::Texture2D::PixelFormat::BGRA8888:
                case cocos2d::Texture2D::PixelFormat::RGBA8888:
                {
                    type = CV_8UC4;
                }
                    break;
                case cocos2d::Texture2D::PixelFormat::RGB888:
                {
                    type = CV_8UC3;
                }
                    break;
                case cocos2d::Texture2D::PixelFormat::RGB565:
                case cocos2d::Texture2D::PixelFormat::RGBA4444:
                {
                    type = CV_8UC2;
                }
                    break;
                case cocos2d::Texture2D::PixelFormat::A8:
                case cocos2d::Texture2D::PixelFormat::I8:
                {
                    type = CV_8UC1;
                }
                    break;
                default:
                    break;
            }

            int imageWidth = srcImage->getWidth();
            int imageHeight = srcImage->getHeight();
            cv::Mat src(imageHeight, imageWidth, type, srcImage->getData());
            cv::Mat dst;
            cv::Rect rect;

            CC_SAFE_DELETE(srcImage);
            srcImage = nullptr;
            if (src.data==nullptr || src.cols<=0 ||src.rows<=0) {
                LOGD("[[[[[ Failed to load thumbnail from android local file 1");
                break;
            }

            data.fastSet(nullptr, 0);
            data.clear();

            int sideLength;

            if (imageHeight > imageWidth) {
                int offset = (imageHeight-imageWidth) / 2;
                rect = cv::Rect(0, offset, imageWidth, imageWidth);
                sideLength = imageWidth;
                //LOGD("[[[[[ procLoadFromAndroidThumbnailThread  8-1");
            } else {
                int offset = (imageWidth-imageHeight) / 2;
                rect = cv::Rect(offset, 0, imageHeight, imageHeight);
                sideLength = imageHeight;
                //LOGD("[[[[[ procLoadFromAndroidThumbnailThread  8-2");
            }

            sideLength = MAX_WIDTH;
            cv::resize(src(rect), dst, cv::Size(sideLength, sideLength), 0, 0, CV_INTER_LINEAR);
            if (dst.data==nullptr || dst.cols<=0 || dst.rows<=0) {
                LOGD("[[[[[ Failed to load thumbnail from android local file 2");
                break;
            }

            switch (format) {
                case cocos2d::Texture2D::PixelFormat::BGRA8888:
                {
                    cv::cvtColor(dst, dst, CV_BGRA2RGBA);
                }
                    break;
                case cocos2d::Texture2D::PixelFormat::RGB888:
                {
                    cv::cvtColor(dst, dst, CV_RGB2RGBA);
                }
                    break;
            }

            int channel = 8*4;
            auto image = new cocos2d::Image();
            if (image->initWithRawData(dst.data, sideLength*sideLength*4, sideLength, sideLength, channel, true)) {
                
                _imageEntry = ImageCacheEntry::createEntry(image, nullptr);
                
                

                dst.release();
                src.release();
            } else {
                LOGD("[[[[[ Failed to Read Image from file !!!!!!!!");
                dst.release();
                src.release();
                CC_SAFE_RELEASE_NULL(image);
                break;
            }

        } else {
            
            LOGD("[[[[[ reading from memory!!!!!!!!");
            _imageEntry = ImageCacheEntry::createEntry(srcImage, nullptr);
        }

        _downloader->handleState(shared_from_this(), ImageDownloader::State::DECODE_SUCCESS);
        return;
        */

    } while (0);
    LOGD("[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[ android thumnail read fail ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]");
    
    _cacheEntry.reset();
    _imageEntry.reset();
    _downloader->handleState(shared_from_this(), ImageDownloader::State::DOWNLOAD_FAILED);
}

#endif
// decode thread func
void DownloadTask::procDecodeThread()
{
    
    _imageEntry.reset();
    
    if (_cacheEntry==nullptr) {
        
        _cacheEntry = *(_downloader->getMemCache()->get(_cacheKey));
        if (_cacheEntry==nullptr) {
            
            CCLOG("WHAT???????? The FXXX");
            _downloader->handleState(shared_from_this(), ImageDownloader::State::DECODE_FAILED);
            return;
        }
    }
    
    do {
        __CHECK_THREAD_INTERRUPTED__;
        
        auto srcImage = new cocos2d::Image();
        
        if (!srcImage->initWithImageData(_cacheEntry->getData(), _cacheEntry->size())) {
            // Failed to decoding
            CC_SAFE_RELEASE_NULL(srcImage);
            CCLOG("Failed to Decoding image");
            
            _downloader->getMemCache()->remove(_cacheKey);
            break;
        }
        
        if (_config._resamplePolicy == DownloadConfig::ResamplePolicy::NONE) {
            
            
            cocos2d::PolygonInfo * polyInfo = nullptr;
            if (_config._enablePhysicsBody) {
                polyInfo = _downloader->makePolygonInfo(srcImage, _config._physicsBodyEpsilon, _config._physicsBodyThreshold);
                if (polyInfo && polyInfo->getVertCount()<3) {
                    CC_SAFE_DELETE(polyInfo);
                }
            }
            
            _imageEntry = ImageCacheEntry::createEntry(srcImage, polyInfo);
            
            
            _downloader->handleState(shared_from_this(), ImageDownloader::State::DECODE_SUCCESS);
            _cacheEntry.reset();
            return;
        } else {
            
            __CHECK_THREAD_INTERRUPTED__;
            
            auto resampleImage = getResampleImage(&srcImage);
            
            if (resampleImage==nullptr) {
                
                CC_SAFE_RELEASE_NULL(srcImage);
                
                
                _downloader->getMemCache()->remove(_cacheKey);
                break;
            }
            
            if (resampleImage != srcImage) {
                CC_SAFE_RELEASE_NULL(srcImage);
            }
            
            cocos2d::PolygonInfo* polyInfo = nullptr;
            if (_config._enablePhysicsBody) {
                polyInfo = _downloader->makePolygonInfo(resampleImage, _config._physicsBodyEpsilon, _config._physicsBodyThreshold);
                if (polyInfo && polyInfo->getVertCount()<3) {
                    CC_SAFE_DELETE(polyInfo);
                }
            }
            
            _imageEntry = ImageCacheEntry::createEntry(resampleImage, polyInfo);
            
            
            _downloader->handleState(shared_from_this(), ImageDownloader::State::DECODE_SUCCESS);
            _cacheEntry.reset();
            return;
        }
    } while(0);
    
    
    _downloader->handleState(shared_from_this(), ImageDownloader::State::DECODE_FAILED);
    _cacheEntry.reset();
}

cocos2d::Image* DownloadTask::getResampleImage(cocos2d::Image** srcImage)
{
    int srcWidth = (*srcImage)->getWidth();
    int srcHeight = (*srcImage)->getHeight();
    
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    float scaleWidth, scaleHeight;
    
    switch (_config._resamplePolicy) {
        case DownloadConfig::ResamplePolicy::NONE:
        {
            
            CC_ASSERT(false);
        }
            break;
        case DownloadConfig::ResamplePolicy::AREA:
        {
            scaleX = scaleY = (_config._resParam1 * _config._resParam2)/(srcWidth*srcHeight);
        }
            break;
        case DownloadConfig::ResamplePolicy::EXACT_FIT:
        {
            scaleX = _config._resParam1 / srcWidth;
            scaleY = _config._resParam2 / srcHeight;
        }
            break;
        case DownloadConfig::ResamplePolicy::EXACT_CROP:
        {
            scaleX = scaleY = std::max(_config._resParam1/srcWidth, _config._resParam2/srcHeight);
        }
            break;
        case DownloadConfig::ResamplePolicy::LONGSIDE:
        {
            if (srcWidth>srcHeight) {
                scaleX = scaleY = _config._resParam1 / srcWidth;
            } else {
                scaleX = scaleY = _config._resParam1 / srcHeight;
            }
        }
            break;
        case DownloadConfig::ResamplePolicy::SCALE:
        {
            scaleX = scaleY = _config._resParam1;
        }
            break;
    }
    
    if (scaleX<=0 || scaleY<=0) {
        return *srcImage;
    }
    
    scaleWidth = srcWidth * scaleX;
    scaleHeight = srcHeight * scaleY;
    
    if (_config._resampleShrinkOnly && srcWidth*srcHeight<=scaleWidth*scaleHeight) {
        return *srcImage;
    }
    
    // apply to interpolation for opencv
    int interpolation;
    switch (_config._resampleMethod) {
        case DownloadConfig::ResampleMethod::ResampleMethod_CUBIC:
        {
            interpolation = CV_INTER_CUBIC;
        }
            break;
        case DownloadConfig::ResampleMethod::ResampleMethod_LANCZOS:
        {
            interpolation = CV_INTER_LANCZOS4;
        }
            break;
        case DownloadConfig::ResampleMethod::ResampleMethod_LINEAR:
        {
            interpolation = CV_INTER_LINEAR;
        }
            break;
        default:
        {
            interpolation = CV_INTER_NN;
        }
            break;
    }
    
    int type = CV_8UC4;
    switch ((*srcImage)->getRenderFormat()) {
        case cocos2d::Texture2D::PixelFormat::BGRA8888:
        case cocos2d::Texture2D::PixelFormat::RGBA8888:
        {
            type = CV_8UC4;
        }
            break;
        case cocos2d::Texture2D::PixelFormat::RGB888:
        {
            type = CV_8UC3;
        }
            break;
        case cocos2d::Texture2D::PixelFormat::RGB565:
        case cocos2d::Texture2D::PixelFormat::RGBA4444:
        {
            type = CV_8UC2;
        }
            break;
        case cocos2d::Texture2D::PixelFormat::A8:
        case cocos2d::Texture2D::PixelFormat::I8:
        {
            type = CV_8UC1;
        }
            break;
        default:
            break;
    }
    
    int dstWidth = (int)scaleWidth;
    int dstHeight = (int)scaleHeight;
    cocos2d::Image* newImage = nullptr;
    

   if (_config._resamplePolicy == DownloadConfig::ResamplePolicy::EXACT_CROP) {
   //if (0) {
///*
        // android local image???
        

        cv::Mat src(srcHeight, srcWidth, type, (*srcImage)->getData());
        cv::Mat dst;
        cv::Rect rect;
        int sideLength;
        if (srcHeight > srcWidth) {
            
            int offset = (srcHeight-srcWidth) / 2;
            rect = cv::Rect(0, offset, srcWidth, srcWidth);
            sideLength = srcWidth;
        } else {
            
            int offset = (srcWidth-srcHeight) / 2;
            rect = cv::Rect(offset, 0, srcHeight, srcHeight);
            sideLength = srcHeight;
        }

        if (sideLength>MAX_WIDTH) {
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
            LOGD("[[[[[ resize small");
#endif
            
            src(rect).copyTo(dst);
            cv::resize(dst, dst, cv::Size(MAX_WIDTH, MAX_WIDTH), 0, 0, CV_INTER_LINEAR);
        } else {
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
            LOGD("[[[[[ resize origin");
#endif
            src(rect).copyTo(dst);
        }
        //cv::resize(src, dst, cv::Size(dstWidth, dstHeight), 0, 0, interpolation);

#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
        LOGD("[[[[[ resize : (%d, %d) -> (%d, %d)", src.rows, src.cols, dst.rows, dst.cols);
#endif

        //CC_SAFE_RELEASE_NULL((*srcImage));


        //if (type!=CV_8UC4) {
//            cv::cvtColor(dst, dst, CV_BGR2RGBA);
//        }

        newImage = new cocos2d::Image();
        newImage->initWithRawData(dst.data, dstWidth*dstHeight*4, dstWidth, dstHeight, 8*4, true);

        dst.release();
        src.release();
//*/
/*
        LOGD("[[[[[ crop 1 ");

        
        cv::Mat src(srcHeight, srcWidth, type, (*srcImage)->getData());
        cv::Mat dst;
        cv::Rect rect;
        int sideLength;

        if (srcHeight > srcWidth) {
            LOGD("[[[[[ crop 1-1 ");
            
            int offset = (srcHeight-srcWidth) / 2;
            rect = cv::Rect(0, offset, srcWidth, srcWidth);
            sideLength = srcWidth;
        } else {
            LOGD("[[[[[ crop 1-2 ");
            
            int offset = (srcWidth-srcHeight) / 2;
            rect = cv::Rect(offset, 0, srcHeight, srcHeight);
            sideLength = srcHeight;
        }

        LOGD("[[[[[ crop 2 ");
        if (sideLength>MAX_WIDTH) {
            LOGD("[[[[[ crop 2-1 (%d, %d)", src.rows, src.cols);
            
            src(rect).copyTo(dst);
            LOGD("[[[[[ crop 2-1-1 before resize : (%d, %d)", dst.rows, dst.cols);
            cv::resize(dst, dst, cv::Size(MAX_WIDTH, MAX_WIDTH), 0, 0, CV_INTER_LINEAR);
            LOGD("[[[[[ crop 2-1-2 after resize : (%d, %d)", dst.rows, dst.cols);
        } else {
            LOGD("[[[[[ crop 2-2 ");
            src(rect).copyTo(dst);
            LOGD("[[[[[ crop 2-2-1 ");
        }

        LOGD("[[[[[ crop 3 (%d, %d)", dst.rows, dst.cols);
        if (dst.data==nullptr || dst.cols<=0 || dst.rows<=0) {
            
            LOGD("[[[[[[ what????? 1");
            return nullptr;
        }

        switch (type) {
            case CV_8UC4:
            {
                LOGD("[[[[[ type 8UC4 1");
                cv::cvtColor(dst, dst, CV_BGRA2BGR);
                LOGD("[[[[[ type 8UC4 2");
            }
                break;
            case CV_8UC3:
            {
                LOGD("[[[[[ type 8UC3 1");
                //cv::cvtColor(dst, dst, CV_BGRA2BGR);
            }
                break;
            case CV_8UC2:
            {
                LOGD("[[[[[ type 8UC2 1");
                cv::cvtColor(dst, dst, CV_GRAY2BGR);
                LOGD("[[[[[ type 8UC2 2");
            }
                break;
            case CV_8UC1:
            {
                LOGD("[[[[[ type 8UC1 ");
            }
                break;
            default:
            {
                LOGD("[[[[[ what type : %d", type);
            }
                break;
        }

        cv::cvtColor(dst, dst, CV_BGR2RGBA);

        LOGD("[[[[[ crop 4 : %d ", sideLength);
        
        newImage= new cocos2d::Image();
        int bytes = dst.total() * dst.elemSize();
        LOGD("[[[[[ crop 5 : %d", bytes);
        //newImage->initWithRawData(dst.data, sideLength*sideLength*4, sideLength, sideLength, 32, true);

        newImage->initWithImageFile("images/MagnifyingGlass.png");
        LOGD("[[[[[ crop image width : %d, height : %d", newImage->getWidth(), newImage->getHeight());

        try {
            auto tmpImage = new cocos2d::Image();
            tmpImage->initWithRawData(dst.data, sideLength*sideLength*4, sideLength, sideLength, 32, true);
            CC_SAFE_DELETE(tmpImage);

        } catch (std::exception e) {
            LOGD("[[[[[ Exception occurred~~~");
        }


        dst.release();
        src.release();
//        */
    } else {
        cv::Mat src(srcHeight, srcWidth, type, (*srcImage)->getData());
        cv::Mat dst;

        cv::resize(src, dst, cv::Size(dstWidth, dstHeight), 0, 0, interpolation);
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
        LOGD("[[[[[ resize aaaaaaaa : (%d, %d) -> (%d, %d)", src.rows, src.cols, dst.rows, dst.cols);
#endif

        CC_SAFE_RELEASE_NULL((*srcImage));
        
        if (type!=CV_8UC4) {
            cv::cvtColor(dst, dst, CV_BGR2RGBA);
        }

        newImage = new cocos2d::Image();
        newImage->initWithRawData(dst.data, dstWidth*dstHeight*4, dstWidth, dstHeight, 8*4, true);

        dst.release();
        src.release();
    }

    CC_SAFE_DELETE((*srcImage));
    (*srcImage) = nullptr;

    return newImage;
}

std::shared_ptr<FileCacheWriteTask> FileCacheWriteTask::createTaskForCache(std::string &cacheKey, std::shared_ptr<MemoryCacheEntry> cacheEntry)
{
    std::shared_ptr<FileCacheWriteTask> task = std::make_shared<FileCacheWriteTask>();
    
    task->_cacheKey = cacheKey;
    task->_cacheEntry = cacheEntry;
    return task;
}

// file cache write thread func
void FileCacheWriteTask::procFileCacheWriteThread()
{
    FileManager::getInstance()->writeToFile(FileManager::FileType::Image, _cacheKey, (u_char*)_cacheEntry->getData(), _cacheEntry->size());
}

