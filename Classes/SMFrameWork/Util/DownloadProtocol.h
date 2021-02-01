//
//  DownloadProtocol.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 15..
//
// Download protocol

#ifndef DownloadProtocol_h
#define DownloadProtocol_h

#include <2d/CCSprite.h>

class DownloadTask;

class DownloadProtocol {
public:
    enum DownloadStartState {
        DOWNLOAD,
        MEM_CACHE,
        IMAGE_CACHE
    };
    
    DownloadProtocol() {
        _downloadTargetAlive = std::make_shared<bool>(true);
    }
    
    virtual ~DownloadProtocol() {
        resetDownload();
    }
    
    virtual void onImageLoadComplete(cocos2d::Sprite* sprite, int tag, bool direct) {}
    virtual void onImageCacheComplete(bool success, int tag) {}
    virtual void onPolygonInfoComplete(cocos2d::PolygonInfo* polyInfo, int tag) {}
    virtual void onImageLoadStart(DownloadStartState state) {}
    
    virtual void onDataLoadComplete(uint8_t* data, ssize_t size, int tag) {}
    virtual void onDataLoadStart(DownloadStartState state) {}
    
    void resetDownload();
    
protected:
    void removeDownloadTask(std::shared_ptr<DownloadTask> task);
    bool isDownloadRunning(const std::string& requestPath, int requestTag);
    bool addDownloadTask(std::shared_ptr<DownloadTask> task);
    
protected:
    std::vector<std::weak_ptr<DownloadTask>> _downloadTask;
    std::shared_ptr<bool> _downloadTargetAlive;
    friend class DownloadTask;
    friend class ImageDownloader;
};


#endif /* DownloadProtocol_h */
