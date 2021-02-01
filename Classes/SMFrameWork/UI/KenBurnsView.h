//
//  KenBurnsView.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 12. 2..
//
// referenced Android kenburnsview opensource https://github.com/flavioarfaria/KenBurnsView

#ifndef KenBurnsView_h
#define KenBurnsView_h

#include "../Base/SMView.h"
#include "../Util/ImageDownloader.h"
#include "../Util/DownloadProtocol.h"
#include <cocos2d.h>

class ShapeSolidRect;

class KenBurnsView : public SMView, public DownloadProtocol
{
public:
    static KenBurnsView* createWithFiles(const std::vector<std::string>& fileList);
    static KenBurnsView* createWithURLs(const std::vector<std::string>& urlList);
    
    virtual void onImageLoadComplete(cocos2d::Sprite* sprite, int tag, bool direct) override;
    virtual void setContentSize(const cocos2d::Size& contentSize) override;
    void startWithDelay(bool delay);
    
    void pauseKenBurns();
    void resumeKenBurns();
    
protected:
    enum Mode {
        FILE,
        URL
    };
    
    KenBurnsView();
    virtual ~KenBurnsView();
    
    bool initWithImageList(Mode mode, const std::vector<std::string>& imageList);
    
private:
    void onNextTransition(float dt);
    
    cocos2d::Rect generateRandomRect(const cocos2d::Size& imageSize);
    
private:
    Mode _mode;
    size_t _sequence;
    int _serial;
    bool _runnable;
    
    std::vector<std::string> _imageList;
    
    ShapeSolidRect* _dimLayer;
};

#endif /* KenBurnsView_h */
