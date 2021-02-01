//
//  ImageDownloader.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 16..
//
//

#include "ImageDownloader.h"
#include "ImageDownloaderThreadPool.h"
#include "DownloadTask.h"
#include "DownloadProtocol.h"
#include <base/CCDirector.h>
#include <base/CCScheduler.h>
#include <physics/CCPhysicsBody.h>
#include "../Base/ImageAutoPolygon.h"

#include "use_opencv.h"

#include "TimerUtil.h"
#include <functional>

const int ImageDownloader::MEM_CACHE_SIZE = (2*1024*1024); //file memory cache 2MByte
const int ImageDownloader::IMAGE_CACHE_SIZE = (4*1080*1920); // one image cache size
const int ImageDownloader::CORE_POOL_SIZE = 8; // (8 threads)
const int ImageDownloader::MAXIMUM_POOL_SIZE = 8;
const DownloadConfig ImageDownloader::DC_DEFAULT = DownloadConfig(DownloadConfig::CachePolycy::DEFAULT);
const DownloadConfig ImageDownloader::DC_NO_CACHE = DownloadConfig(DownloadConfig::CachePolycy::NO_CACHE);
const DownloadConfig ImageDownloader::DC_NO_DISK = DownloadConfig(DownloadConfig::CachePolycy::NO_DISK);
const DownloadConfig ImageDownloader::DC_NO_IMAGE = DownloadConfig(DownloadConfig::CachePolycy::NO_IMAGE);
const DownloadConfig ImageDownloader::DC_CACHE_ONLY = DownloadConfig(DownloadConfig::CachePolycy::DEFAULT, true);
const DownloadConfig ImageDownloader::DC_CACHE_ONLY_NO_IMAGE = DownloadConfig(DownloadConfig::CachePolycy::NO_IMAGE, true);
const DownloadConfig ImageDownloader::DC_CACHE_ONLY_NO_DISK = DownloadConfig(DownloadConfig::CachePolycy::NO_DISK, true);
const DownloadConfig ImageDownloader::DC_CACHE_ONLY_DISK_ONLY = DownloadConfig(DownloadConfig::CachePolycy::DISK_ONLY, true);
const DownloadConfig ImageDownloader::DC_IMAGE_ONLY = DownloadConfig(DownloadConfig::CachePolycy::IMAGE_ONLY, true);

ImageDownloader& ImageDownloader::getInstance()
{
    static ImageDownloader instance(MEM_CACHE_SIZE, MEM_CACHE_SIZE, 4, 4);
    return instance;
}


ImageDownloader& ImageDownloader::getInstanceForFitting()   //// what is this????
{
    static ImageDownloader instance(MEM_CACHE_SIZE, (32*1024*1024), 4, 4);
    return instance;
}

DownloadConfig::DownloadConfig(const CachePolycy cachePolicy, const bool cacheOnly) :
_cachePolicy(cachePolicy),
_resamplePolicy(ResamplePolicy::NONE),
_resampleMethod(ResampleMethod::ResampleMethod_LINEAR),
_resampleShrinkOnly(true),
_resParam1(0),
_resParam2(0),
_reqDegress(0),
_enablePhysicsBody(false),
_cacheOnly(cacheOnly),
_smallThumbnail(false)
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
, _maxSideLength(1280)
#endif
{
    
}

void DownloadConfig::setCachePolicy(const DownloadConfig::CachePolycy cachePolicy)
{
    _cachePolicy = cachePolicy;
}

#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
void DownloadConfig::setMaxSideLength(int maxSideLength)
{
    if (maxSideLength > 0) {
        _maxSideLength = maxSideLength;
    }
}
#endif

void DownloadConfig::setResamplePolicy(const DownloadConfig::ResamplePolicy resamplePolicy, const float param1, const float param2)
{
    _resamplePolicy = resamplePolicy;
    _resParam1 = param1;
    _resParam2 = param2;
    
    switch (_resamplePolicy) {
        case DownloadConfig::ResamplePolicy::AREA:
        {
            CCASSERT(_resParam1 > 0 && _resParam2 > 0, "ERROR");
        }
            break;
        case DownloadConfig::ResamplePolicy::EXACT_FIT:
        {
            CCASSERT(_resParam1 > 0 && _resParam2 > 0, "ERROR");
        }
            break;
        case DownloadConfig::ResamplePolicy::EXACT_CROP:
        {
            CCASSERT(_resParam1 > 0 && _resParam2 > 0, "ERROR");
        }
            break;
        case DownloadConfig::ResamplePolicy::LONGSIDE:
        {
            CCASSERT(_resParam1 > 0, "ERROR");
        }
            break;
        case DownloadConfig::ResamplePolicy::SCALE:
        {
            CCASSERT(_resParam1 > 0, "ERROR");
            if (_resParam1 >= 1.0) {
                _resamplePolicy = DownloadConfig::ResamplePolicy::NONE;
            }
        }
            break;
        default:
            break;
    }
}

void DownloadConfig::setResampleMethod(const DownloadConfig::ResampleMethod resampleMethod)
{
    _resampleMethod = resampleMethod;
}

void DownloadConfig::setRotation(const int degress)
{
    _reqDegress = degress;
}

void DownloadConfig::setPhysicsBodyEnable(bool enable, const float epsilon, const float threshold)
{
    _enablePhysicsBody = enable;
    _physicsBodyEpsilon = epsilon;
    _physicsBodyThreshold = threshold;
}

ImageDownloader::ImageDownloader(const int memCacheSize, const int imageCacheSize, const int downloadPoolSize, const int decodePoolSize) :
_memCacheSize(memCacheSize),
_imageCacheSize(imageCacheSize),
_downloadPoolSize(downloadPoolSize),
_decodePoolSize(decodePoolSize),
_memCache(nullptr),
_imageCache(nullptr),
_decodeThreadPool(nullptr),
_downloadThreadPool(nullptr),
_fileCacheWriteThreadPool(nullptr)
{
    init();
}

ImageDownloader::~ImageDownloader()
{
    _decodeThreadPool->interrupt();
    _downloadThreadPool->interrupt();
    _fileCacheWriteThreadPool->interrupt();
    
    CC_SAFE_DELETE(_decodeThreadPool);
    CC_SAFE_DELETE(_downloadThreadPool);
    CC_SAFE_DELETE(_fileCacheWriteThreadPool);
    
    if (_memCache) {
        _memCache->removeAll();
        CC_SAFE_DELETE(_memCache);
    }
    
    if (_imageCache) {
        _imageCache->removeAll();
        CC_SAFE_DELETE(_imageCache);
    }
    
    
}

void ImageDownloader::init()
{
    _memCache = new MemoryLRUCache(_memCacheSize);
    _imageCache = new ImageLRUCache(_imageCacheSize);
    _decodeThreadPool = new ThreadPool(true, _decodePoolSize);
    _downloadThreadPool = new ThreadPool(true, _downloadPoolSize);
    _fileCacheWriteThreadPool = new ThreadPool(true, 1);
    
    
    
}

void ImageDownloader::queueDownloadTask(DownloadProtocol *target, const std::shared_ptr<DownloadTask> &task)
{
    if (task->getConfig().isEnableImageCache()) {
        std::string key = task->getCacheKey();
        auto imageEntry = _imageCache->get(key);
        
        if (imageEntry) {
            auto image = (*imageEntry)->getImage();
            if (image && image->getData()) {
                
                target->onImageLoadStart(DownloadProtocol::DownloadStartState::IMAGE_CACHE);
                handleState(task, State::IMAGE_CACHE_DIRECT, imageEntry);
                return;
            } else {
                
                _imageCache->remove(key);
            }
        }
    }
    
    if (task->getConfig().isEnableMemoryCache()) {
        auto memoryEtnry = _memCache->get(task->getCacheKey());
        
        if (memoryEtnry && (*memoryEtnry)->size()>0) {
            task->setMemoryCacheEntry(memoryEtnry);
        }
    }
    
    if (task->getMemoryCacheEntry()) {
        
        target->onImageLoadStart(DownloadProtocol::DownloadStartState::MEM_CACHE);
        handleState(task, State::DECODE_STARTED);
    } else {
        
        target->onImageLoadStart(DownloadProtocol::DownloadStartState::DOWNLOAD);
        handleState(task, State::DOWNLOAD_STARTED);
    }
}

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
// NSURL download for asset
void ImageDownloader::loadImageFromLocalIdentifier(DownloadProtocol *target, const std::string &requestUrl, const int tag, DownloadConfig* config)
{
    CCASSERT(false, "DEPRECATE");
//    CCASSERT(target!=nullptr, "Invalid download parameter");
//
//    if (requestUrl.length()==0) {
//        target->onImageLoadComplete(nullptr, tag, true);
//    }
//
//    std::shared_ptr<DownloadTask> task = DownloadTask::createTaskForTarget(this, target);
//    task->init(DownloadTask::MediaType::IOSIMAGE, requestUrl, config);
//    task->setTag(tag);
//
//    if (target->addDownloadTask(task)) {
//        queueDownloadTask(target, task);
//    }
}

// NSURL download for asset thumbnail
void ImageDownloader::loadThumbnailFromLocalIdentifier(DownloadProtocol *target, const std::string &requestUrl, const int tag, DownloadConfig* config)
{
    CCASSERT(false, "DEPRECATE");
//    CCASSERT(target!=nullptr, "Invalid download parameter");
//
//    if (requestUrl.length()==0) {
//        target->onImageLoadComplete(nullptr, tag, true);
//    }
//
//    std::shared_ptr<DownloadTask> task = DownloadTask::createTaskForTarget(this, target);
//    task->init(DownloadTask::MediaType::IOSTHUMBNAIL, requestUrl, config);
//    task->setTag(tag);
//
//    if (target->addDownloadTask(task)) {
//        queueDownloadTask(target, task);
//    }
}

void ImageDownloader::loadImagePHAssetThumbnail(DownloadProtocol* target, void* phAssetObject, const int tag, DownloadConfig* config)
{
    CCASSERT(target != nullptr, "INVALID DOWNLOAD PARAM");
    
    if (phAssetObject == nullptr) {
        target->onImageLoadComplete(nullptr, tag, true);
        return;
    }
    
    
    std::shared_ptr<DownloadTask> task = DownloadTask::createTaskForTarget(this, target);
    task->setPHAssetObject(phAssetObject);
    task->init(DownloadTask::MediaType::IOSTHUMBNAIL, task->getPHAssetIdentifier(), config);
    task->setTag(tag);
    
    if (target->addDownloadTask(task)) {
        queueDownloadTask(target, task);
    }
}

void ImageDownloader::loadImagePHAsset(DownloadProtocol* target, void* phAssetObject, const int tag, DownloadConfig* config)
{
    CCASSERT(target != nullptr, "INVALID DOWNLOAD PARAM");
    
    if (phAssetObject == nullptr) {
        target->onImageLoadComplete(nullptr, tag, true);
        return;
    }

    std::shared_ptr<DownloadTask> task = DownloadTask::createTaskForTarget(this, target);
    task->setPHAssetObject(phAssetObject);
    task->init(DownloadTask::MediaType::IOSIMAGE, task->getPHAssetIdentifier(), config);
    task->setTag(tag);
    
    if (target->addDownloadTask(task)) {
        queueDownloadTask(target, task);
    }
}

#elif CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID

void ImageDownloader::loadImageFromAndroidLocalFile(DownloadProtocol *target, const std::string &requestUrl, const int tag, DownloadConfig* config)
{
    CCASSERT(target!=nullptr, "Invalid download parameter");
    if (requestUrl.length()==0) {
        target->onImageLoadComplete(nullptr, tag, true);
    }

    if (target->isDownloadRunning(requestUrl, tag)) {
        return;
    }

    std::shared_ptr<DownloadTask> task = DownloadTask::createTaskForTarget(this, target);
    task->init(DownloadTask::MediaType::ANDROIDIMAGE, requestUrl, config);
    task->setTag(tag);

    if (target->addDownloadTask(task)) {
        queueDownloadTask(target, task);
    }
}

void ImageDownloader::loadImageFromAndroidThumbnail(DownloadProtocol *target, const std::string &requestUrl, const int tag, DownloadConfig* config)
{
    CCASSERT(target!=nullptr, "Invalid download parameter");
    if (requestUrl.length()==0) {
        target->onImageLoadComplete(nullptr, tag, true);
    }

    if (target->isDownloadRunning(requestUrl, tag)) {
        return;
    }

    std::shared_ptr<DownloadTask> task = DownloadTask::createTaskForTarget(this, target);
    task->init(DownloadTask::MediaType::ANDROIDTHUMB, requestUrl, config);
    task->setTag(tag);

    if (target->addDownloadTask(task)) {
        queueDownloadTask(target, task);
    }
}

#endif

// download for network
void ImageDownloader::loadImageFromNetwork(DownloadProtocol *target, const std::string &requestUrl, const int tag, DownloadConfig* config)
{
    CCASSERT(target!=nullptr, "Invalid download parameter");
    
    if (requestUrl.length()==0) {
        target->onImageLoadComplete(nullptr, tag, true);
        return;
    }
    
    if (target->isDownloadRunning(requestUrl, tag)) {
        return;
    }
    
    std::shared_ptr<DownloadTask> task = DownloadTask::createTaskForTarget(this, target);
    task->init(DownloadTask::MediaType::NETWORK, requestUrl, config);
    task->setTag(tag);
    
    if (target->addDownloadTask(task)) {
        queueDownloadTask(target, task);
    }
}

// download for app resource
void ImageDownloader::loadImageFromResource(DownloadProtocol *target, const std::string &requestUrl, const int tag, DownloadConfig* config)
{
    CCASSERT(target!=nullptr, "Invalid download parameter");
    
    if (requestUrl.length()==0) {
        target->onImageLoadComplete(nullptr, tag, true);
    }
    
    if (target->isDownloadRunning(requestUrl, tag)) {
        return;
    }
    
    /*
    
    auto texture = cocos2d::Director::getInstance()->getTextureCache()->getTextureForKey(requestUrl);
    if (texture) {
        auto sprite = cocos2d::Sprite::createWithTexture(texture);
        target->onImageLoadComplete(sprite, tag, true);
        return;
    }
    */
    
    std::shared_ptr<DownloadTask> task = DownloadTask::createTaskForTarget(this, target);
    task->init(DownloadTask::MediaType::RESOURCE, requestUrl, config);
    task->setTag(tag);
    
    if (target->addDownloadTask(task)) {
        queueDownloadTask(target, task);
    }
}

// download for local file (device or app documents...)
void ImageDownloader::loadImageFromFile(DownloadProtocol *target, const std::string &requestUrl, const int tag, DownloadConfig* config)
{
    CCASSERT(target!=nullptr, "Invalid download parameter");
    if (requestUrl.length()==0) {
        target->onImageLoadComplete(nullptr, tag, true);
    }
    
    if (target->isDownloadRunning(requestUrl, tag)) {
        return;
    }
    
    std::shared_ptr<DownloadTask> task = DownloadTask::createTaskForTarget(this, target);
    task->init(DownloadTask::MediaType::FILE, requestUrl, config);
    task->setTag(tag);
    
    if (target->addDownloadTask(task)) {
        queueDownloadTask(target, task);
    }
}

void ImageDownloader::cancelImageDownload(DownloadProtocol *target)
{
    if (target) {
        target->resetDownload();
    }
}

bool ImageDownloader::isCachedForNetwork(const std::string &requestPath, DownloadConfig *config) const {
    auto cacheKey = DownloadTask::makeCacheKey(DownloadTask::MediaType::NETWORK, requestPath, config, nullptr);
    auto image = _imageCache->get(cacheKey);
    
    if (image) {
        return true;
    }
    
    return false;
}

bool ImageDownloader::saveToFileCache(const std::string &requestPath, uint8_t *data, ssize_t length, DownloadConfig* config)
{
    // not used
    return true;
}

static const cocos2d::PhysicsMaterial PHYSICSSHAPE_MATERIAL_ZERO = cocos2d::PhysicsMaterial(0.0, 0.0, 0.0);

cocos2d::PhysicsBody* ImageDownloader::polygonInfoToPhysicsBody(const cocos2d::PolygonInfo *info, const cocos2d::Size &imageSize)
{
    if (info==nullptr) {
        return nullptr;
    }
    
    auto body = cocos2d::PhysicsBody::create();
    auto offset = cocos2d::Size(imageSize.width/2, imageSize.height/2);
    
    cocos2d::V3F_C4B_T2F* verts = info->triangles.verts;
    auto indices  = info->triangles.indices;
    auto indexCount = info->triangles.indexCount;
    
    cocos2d::Vec2 triPoint[3];
    
    cocos2d::Vec3* v;
    for (ssize_t index=0; index<indexCount; index+=3) {
        auto i0 = *(indices++);
        auto i1 = *(indices++);
        auto i2 = *(indices++);
        
        v = &((verts + i0)->vertices); triPoint[2].x = v->x-offset.width; triPoint[2].y = v->y-offset.height;
        v = &((verts + i1)->vertices); triPoint[1].x = v->x-offset.width; triPoint[1].y = v->y-offset.height;
        v = &((verts + i2)->vertices); triPoint[0].x = v->x-offset.width; triPoint[0].y = v->y-offset.height;
        
        auto shape = cocos2d::PhysicsShapePolygon::create(triPoint, 3, PHYSICSSHAPE_MATERIAL_ZERO, cocos2d::Vec2::ZERO);
        body->addShape(shape);
    }
    
    return body;
}

void ImageDownloader::handleState(std::shared_ptr<DownloadTask> task, ImageDownloader::State state, std::shared_ptr<ImageCacheEntry>* imageEntry)
{
    switch (state) {
        case State::DOWNLOAD_STARTED:
        {
            // network download start
            std::lock_guard<std::mutex> guard(_mutex_download);
            
            switch (task->getMediaType()) {
                case DownloadTask::MediaType::NETWORK :
                    addDecodeTask(std::bind(&DownloadTask::procDownloadThread, task));
                    break;
                case DownloadTask::MediaType::RESOURCE:
                    addDecodeTask(std::bind(&DownloadTask::procLoadFromResourceThread, task));
                    break;
                case DownloadTask::MediaType::FILE:
                    addDecodeTask(std::bind(&DownloadTask::procLoadFromFileThread, task));
                    break;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
                case DownloadTask::MediaType::IOSIMAGE:
                    addDecodeTask(std::bind(&DownloadTask::procLoadFromLocalIdentifierThread, task));
                    break;
                case DownloadTask::MediaType::IOSTHUMBNAIL:
                    addDecodeTask(std::bind(&DownloadTask::procLoadFromLocalIdentifierThumbnailThread, task));
                    break;
#elif CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
                case DownloadTask::MediaType::ANDROIDIMAGE:
                    addDecodeTask(std::bind(&DownloadTask::procLoadFromAndroidLocalFileThread, task));
                    break;
                case DownloadTask::MediaType::ANDROIDTHUMB:
                    addDecodeTask(std::bind(&DownloadTask::procLoadFromAndroidThumbnailThread, task));
                    break;
#endif
                default:
                    break;
            }
        }
            break;
        case State::DOWNLOAD_SUCCESS:
        {
            std::lock_guard<std::mutex> guard(_mutex_download);
            if (task->getConfig().isCacheOnly() && !task->getConfig().isEnableImageCache()) {
                
                if (task->isTargetAlive()) {
                    auto scheduler = cocos2d::Director::getInstance()->getScheduler();
                    scheduler->performFunctionInCocosThread([task]{
                        if (task->isTargetAlive()) {
                            task->getTarget()->onImageCacheComplete(true, task->getTag());
                            task->getTarget()->removeDownloadTask(task);
                        }
                    });
                }
            } else {
                if (task->isTargetAlive()) {
                    
                    handleState(task, ImageDownloader::State::DECODE_STARTED);
                }
            }
        }
            break;
        case State::DOWNLOAD_FAILED:
        {
            std::lock_guard<std::mutex> guard(_mutex_download);
            
            auto imageEntry = task->getImageCacheEntry();
            if (imageEntry) {
                imageEntry.reset();
            }
            
            if (task->isTargetAlive()) {
                auto scheduler = cocos2d::Director::getInstance()->getScheduler();
                scheduler->performFunctionInCocosThread([task]{
                    if (task->isTargetAlive()) {
                        
                        if (task->getConfig().isCacheOnly()) {
                            task->getTarget()->onImageCacheComplete(true, task->getTag());
                        } else {
                            task->getTarget()->onImageLoadComplete(nullptr, task->getTag(), false);
                        }
                        task->getTarget()->removeDownloadTask(task);
                    }
                });
            }
        }
            break;
        case State::DECODE_STARTED:
        {
            std::lock_guard<std::mutex> guard(_mutex_decode);
            
            if (task->isTargetAlive()) {
                addDecodeTask(std::bind(&DownloadTask::procDecodeThread, task));
            }
        }
            break;
        case State::DECODE_SUCCESS:
        {
            
            std::lock_guard<std::mutex> guard(_mutex_decode);
            
            if (task->isTargetAlive()) {
                auto scheduler = cocos2d::Director::getInstance()->getScheduler();
                scheduler->performFunctionInCocosThread([task]{
                    auto imageEntry = task->getImageCacheEntry();
                    if (task->getConfig().isEnableImageCache()) {
                        
                        auto imageCache = task->getDownloader()->getImageCache();
                        imageCache->insert(task->getCacheKey(), imageEntry);
                        imageCache->printfDownloadStat("IMG CACHE");
                    }
                    
                    if (task->isTargetAlive()) {
                        if (task->getConfig().isCacheOnly()) {
                            
                            task->getTarget()->onImageCacheComplete(true, task->getTag());
                        } else {
                            
                            auto texture = new cocos2d::Texture2D();
                            texture->initWithImage(imageEntry->getImage());
                            
                            auto sprite = cocos2d::Sprite::createWithTexture(texture);
                            CC_SAFE_RELEASE(texture);
                            
                            task->getTarget()->onImageLoadComplete(sprite, task->getTag(), false);
                            
                            // polygon info for PhysicsBody check
                            if (task->getConfig().isEnablePhysicsBody()) {
                                task->getTarget()->onPolygonInfoComplete(imageEntry->getPolygonInfo(), task->getTag());
                            }
                        }
                        
                        task->getTarget()->removeDownloadTask(task);
                    }
                    
                    imageEntry.reset();
                });
            } else {
                
                auto imageEntry = task->getImageCacheEntry();
                
                if (task->getConfig().isEnableImageCache()) {
                    auto imageCache = task->getDownloader()->getImageCache();
                    imageCache->insert(task->getCacheKey(), imageEntry);
                    imageCache->printfDownloadStat("IMG CACHE");
                }
                
                imageEntry.reset();
            }
        }
            break;
        case State::DECODE_FAILED:
        {
            // failed to decode
            std::lock_guard<std::mutex> guard(_mutex_decode);
            
            if (task->isTargetAlive()) {
                auto scheduler = cocos2d::Director::getInstance()->getScheduler();
                scheduler->performFunctionInCocosThread([task]{
                    if (task->isTargetAlive()) {
                        if (task->getConfig().isCacheOnly()) {
                            task->getTarget()->onImageCacheComplete(false, task->getTag());
                        } else {
                            task->getTarget()->onImageLoadComplete(nullptr, task->getTag(), false); 
                        }
                        task->getTarget()->removeDownloadTask(task);
                    }
                });
            }
        }
            break;
        case State::IMAGE_CACHE_DIRECT:
        {
            
            if (task->getConfig().isCacheOnly()) {
                
                task->getTarget()->onImageCacheComplete(true, task->getTag());
            } else {
                if (task->isTargetAlive()) {
                auto texture = new cocos2d::Texture2D();
                texture->initWithImage((*imageEntry)->getImage());
                
                auto sprite = cocos2d::Sprite::createWithTexture(texture);
                CC_SAFE_RELEASE(texture);
                
                task->getTarget()->onImageLoadComplete(sprite, task->getTag(), false);  // image load from cache succeed
                
                // polygon info for physicsbody
                if (task->getConfig().isEnablePhysicsBody()) {
                    
                    if ((*imageEntry)->getPolygonInfo()==nullptr) {
                            //                        TimerUtil::start();
                        auto polyInfo = makePolygonInfo((*imageEntry)->getImage(), task->getConfig()._physicsBodyEpsilon, task->getConfig()._physicsBodyThreshold);
                            //                        TimerUtil::stop("POLYGON");
                        if (polyInfo) {
                            (*imageEntry)->setPolygonInfo(polyInfo);
                        }
                    }
                    task->getTarget()->onPolygonInfoComplete((*imageEntry)->getPolygonInfo(), task->getTag());
                }
                } else {
                    CCLOG("[[[[[ task is not alive!!!!");
                }
            }
            task->getTarget()->removeDownloadTask(task);
        }
            break;
            
        default:
            break;
    }
}


#define SCALED_SIZE (200)
cocos2d::PolygonInfo* ImageDownloader::makePolygonInfo(cocos2d::Image *image, const float epsilon, const float threshold)
{
    if (image==nullptr) {
        return nullptr;
    }
    
    std::lock_guard<std::mutex> guard(_mutex_physics);
    
    int width = image->getWidth();
    int height = image->getHeight();
    
    float scaleX = (float)SCALED_SIZE/width;
    float scaleY = (float)SCALED_SIZE/height;
    
    int type = CV_8UC4;
    switch (image->getRenderFormat()) {
        default:
        {
            // rgba 8888
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
    }
    
    cv::Mat src(height, width, type, image->getData());
    cv::Mat dst;
    
    cv::resize(src, dst, cv::Size(SCALED_SIZE, SCALED_SIZE), 0, 0, CV_INTER_LINEAR);
    cv::circle(dst, cv::Point(SCALED_SIZE/2, SCALED_SIZE/2), SCALED_SIZE/4, cv::Scalar(0xFF, 0xFF, 0xFF, 0xFF), CV_FILLED);
    
    cocos2d::Image* scaledImage = new cocos2d::Image();
    scaledImage->initWithRawData(dst.data, SCALED_SIZE*SCALED_SIZE*4, SCALED_SIZE, SCALED_SIZE, 8*4, true);
    
    ImageAutoPolygon ap(scaledImage);
    auto polyInfo = ap.generateTriangles(cocos2d::Rect::ZERO, epsilon, threshold);
    CC_SAFE_DELETE(scaledImage);
    
    if (polyInfo && (polyInfo->getVertCount()<6 || polyInfo->getArea()<SCALED_SIZE*SCALED_SIZE*0.5)) {
        // vertex가 너무 작거나 영역이 작으면 polygoninfo 해제
        CC_SAFE_DELETE(polyInfo);
        polyInfo = nullptr;
    }
    
    if (polyInfo) {
        // make original size
        auto vert = polyInfo->triangles.verts;
        int count = polyInfo->getVertCount();
        
        for (int i=0; i<count; i++) {
            vert->vertices.x /= scaleX;
            vert->vertices.y /= scaleY;
            vert++;
        }
    }
    
    return polyInfo;
}

void ImageDownloader::writeToFileCache(std::string &cacheKey, std::shared_ptr<MemoryCacheEntry> cacheEntry)
{
    std::lock_guard<std::mutex> guard(_mutex_file);
    
    auto task = FileCacheWriteTask::createTaskForCache(cacheKey, cacheEntry);
    _fileCacheWriteThreadPool->addTask(std::bind(&FileCacheWriteTask::procFileCacheWriteThread, task));
}

void ImageDownloader::addDownloadTask(const std::function<void ()> &task)
{
    _downloadThreadPool->addTask(task);
}

void ImageDownloader::addDecodeTask(const std::function<void ()> &task)
{
    _decodeThreadPool->addTask(task);
}

void ImageDownloader::clearCache()
{
    _imageCache->removeAll();
    _memCache->removeAll();
}




// download protocol implement
void DownloadProtocol::resetDownload()
{
    
    for (auto iter = _downloadTask.begin(); iter!=_downloadTask.end(); ++iter) {
        if (!iter->expired()) {
            auto task = iter->lock();
            if (task != nullptr && task->isRunning()) {
                task->interrupt();
            }
        }
        iter->reset();
    }
    
    
    
    _downloadTask.clear();
    
    
    _downloadTargetAlive.reset();
    _downloadTargetAlive = std::make_shared<bool>(true);
}

void DownloadProtocol::removeDownloadTask(std::shared_ptr<DownloadTask> task)
{
    for (auto iter=_downloadTask.begin(); iter!=_downloadTask.end();) {
        auto t = iter->lock();
        
        if (iter->expired()) {
            
            iter = _downloadTask.erase(iter);
        } else if (task && t && (task==t || task->getCacheKey().compare(t->getCacheKey())==0)) {
            
            task->interrupt();
            iter->reset();
            iter = _downloadTask.erase(iter);
        } else {
            // 다음
            ++iter;
        }
    }
}

bool DownloadProtocol::isDownloadRunning(const std::string &requestPath, int requestTag)
{
    for (auto iter=_downloadTask.begin(); iter!=_downloadTask.end(); ++iter) {
        auto t = iter->lock();
        if (t->getRequestPath().compare(requestPath)==0 && t->getTag()==requestTag) {
            
            return true;
        }
    }
    
    return false;
}

bool DownloadProtocol::addDownloadTask(std::shared_ptr<DownloadTask> task)
{
    for (auto iter=_downloadTask.begin(); iter!=_downloadTask.end();) {
        auto t = iter->lock();
        
        if (iter->expired()) {
            
            iter = _downloadTask.erase(iter);
        } else if (task && t && t->isRunning() && (task==t || task->getCacheKey().compare(task->getCacheKey())==0)) {
            
            return false;
        } else {
            ++iter;
        }
    }
    
    _downloadTask.emplace_back(task);
    return true;
}






