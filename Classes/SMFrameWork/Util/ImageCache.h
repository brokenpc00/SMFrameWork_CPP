//
//  ImageCache.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 15..
//
// Image LRU Cache

#ifndef ImageCache_h
#define ImageCache_h

#include <platform/CCImage.h>
#include <2d/CCAutoPolygon.h>
#include "LRUCache.h"
#include <memory>

class ImageCacheEntry {
public:
    static std::shared_ptr<ImageCacheEntry> createEntry(cocos2d::Image* image, cocos2d::PolygonInfo* polygonInfo=nullptr)
    {
        CCASSERT(image!=nullptr, "image must not nullptr");
        
        std::shared_ptr<ImageCacheEntry> entry = std::make_shared<ImageCacheEntry>();
        entry->_image = image;
        entry->_polygonInfo = polygonInfo;
        
        return entry;
    }
    
    cocos2d::Image* getImage() {return _image;}
    cocos2d::PolygonInfo* getPolygonInfo() {return _polygonInfo;}
    
    void setPolygonInfo(cocos2d::PolygonInfo* polygonInfo) {
        if (_polygonInfo!=polygonInfo) {
            CC_SAFE_DELETE(_polygonInfo);
            _polygonInfo = polygonInfo;
        }
    }
    
    ssize_t size() {
        if (_image) {
            return _image->getDataLen();
        } else {
            return 0;
        }
    }
    
public:
    ImageCacheEntry() :
    _image(nullptr),
    _polygonInfo(nullptr)
    {
        
    }
    
    ~ImageCacheEntry() {
        if (_image) {
            _image->release();
        }
        
        if (_polygonInfo) {
            CC_SAFE_DELETE(_polygonInfo);
        }
    }
    
private:
    cocos2d::Image* _image;
    cocos2d::PolygonInfo* _polygonInfo;
};

class ImageLRUCache : public LRUCache<std::string, std::shared_ptr<ImageCacheEntry>> {
public:
    ImageLRUCache(ssize_t maxCapacity) {
        setMaxCapacity(maxCapacity);
    }
    
private:
    virtual ssize_t sizeOf(const std::string& key, std::shared_ptr<ImageCacheEntry>& entry) override {
        return entry->size();
    }
};

#endif /* ImageCache_h */
