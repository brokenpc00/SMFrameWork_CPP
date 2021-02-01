//
//  KenBurnsView.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 12. 2..
//
//

#include "KenBurnsView.h"
#include "../Base/ShaderNode.h"
#include "../Base/ViewAction.h"

#define PAN_TIME (10.0f)
#define FADE_TIME (1.5f)

#define MINUM_RECT_FACTOR (0.8f)
#define DIM_LAYER_COLOR cocos2d::Color4F(0, 0, 0, 0.6f)

// transition animation

class TransitionAction : public ViewAction::DelayBaseAction
{
public:
    CREATE_DELAY_ACTION(TransitionAction);
    
    virtual void onStart() override {
        
    }
    
    virtual void onUpdate(float dt) override {
        // pan / zoom
        updateTextureRect(dt);
        
        // alpha
        float time = getDuration() * dt;
        float alpha = std::min(time/FADE_TIME, 1.0f);
        _sprite->setOpacity((GLubyte)(0xff*alpha));
        
        // scale
        _sprite->setScale(_target->getContentSize().width/_sprite->getContentSize().width);
    }
    
    virtual void onEnd() override {
        _target->removeChild(_sprite);
    }
    
    void updateTextureRect(float t) {
        float x = ViewUtil::interpolation(_src.origin.x, _dst.origin.x, t);
        float y = ViewUtil::interpolation(_src.origin.y, _dst.origin.y, t);
        float w = ViewUtil::interpolation(_src.size.width, _dst.size.width, t);
        float h = ViewUtil::interpolation(_src.size.height, _dst.size.height, t);
        _sprite->setTextureRect(cocos2d::Rect(x, y, w, h));
    }
    
    void setValue(cocos2d::Sprite* sprite, const cocos2d::Rect& src, const cocos2d::Rect& dst, float duration, float delay) {
        setTimeValue(duration, delay);
        _sprite = sprite;
        _src = src;
        _dst = dst;
        
        _sprite->setOpacity(0.0f);
        updateTextureRect(0.0f);
    }
    
protected:
    cocos2d::Sprite* _sprite;
    
    cocos2d::Rect _src, _dst;
};





// KenBurnsView
KenBurnsView * KenBurnsView::createWithFiles(const std::vector<std::string> &fileList)
{
    auto view = new (std::nothrow) KenBurnsView();
    
    if (view != nullptr && view->initWithImageList(Mode::FILE, fileList)) {
        view->autorelease();
    } else {
        CC_SAFE_DELETE(view);
    }
    
    return view;
}

KenBurnsView * KenBurnsView::createWithURLs(const std::vector<std::string> &urlList)
{
    auto view = new (std::nothrow) KenBurnsView();
 
    if (view != nullptr && view->initWithImageList(Mode::URL, urlList)) {
        view->autorelease();
    } else {
        CC_SAFE_DELETE(view);
    }
    
    return view;
}

KenBurnsView::KenBurnsView() : _sequence(0), _serial(0), _runnable(true)
{
    
}

KenBurnsView::~KenBurnsView()
{
    
}

bool KenBurnsView::initWithImageList(KenBurnsView::Mode mode, const std::vector<std::string> &imageList)
{
    if (imageList.size()==0) {
        return false;
    }
    
    _mode = mode;
    _imageList = imageList;
    
    _dimLayer = ShapeSolidRect::create();
    _dimLayer->setContentSize(_contentSize);
    _dimLayer->setColor4F(DIM_LAYER_COLOR);
    addChild(_dimLayer);
    
    return true;
}

void KenBurnsView::setContentSize(const cocos2d::Size &contentSize)
{
    SMView::setContentSize(contentSize);
    
    if (_dimLayer) {
        _dimLayer->setContentSize(contentSize);
    }
}

void KenBurnsView::startWithDelay(bool delay)
{
    if (delay<=0) {
        // immediate
        onNextTransition(0);
    } else {
        if (_mode==Mode::URL) {
            // network file인 경우 첫번째 파일을 미리 다운 받는다.
            ImageDownloader::getInstance().loadImageFromNetwork(this, _imageList.at(0), _serial++, (DownloadConfig*)&ImageDownloader::DC_CACHE_ONLY);
        }
        
        // schedule에 등록한다.
        scheduleOnce(CC_SCHEDULE_SELECTOR(KenBurnsView::onNextTransition), delay);
    }
}

void KenBurnsView::onNextTransition(float dt)
{
    if (_mode==Mode::URL) {
        // url은 cache only.. 이미 셋팅되어 있음.
        ImageDownloader::getInstance().loadImageFromNetwork(this, _imageList.at(_sequence), _serial++);
    } else {
        // file은 cache 안함.
        ImageDownloader::getInstance().loadImageFromResource(this, _imageList.at(_sequence), _serial++, (DownloadConfig*)&ImageDownloader::DC_NO_CACHE);
    }
    
    _sequence++;
    // image size만큼 되면 다시 0으로 해야하니까...
    _sequence %= _imageList.size();
}

void KenBurnsView::onImageLoadComplete(cocos2d::Sprite *sprite, int tag, bool direct)
{
    if (sprite==nullptr) {
        return;
    }
    sprite->setPosition(_contentSize/2);
    addChild(sprite);
    
    auto src = generateRandomRect(sprite->getContentSize());
    auto dst = generateRandomRect(sprite->getContentSize());
    
    auto action = TransitionAction::create();
    action->setValue(sprite, src, dst, PAN_TIME, 0);
    action->setTag(17);
    
    runAction(action);
    
    if (_mode==Mode::URL) {
        // network file이면 또 그 다음 파일을 미리 다운 받는다.
        size_t nextSeq = (_sequence+1) % _imageList.size();
        ImageDownloader::getInstance().loadImageFromNetwork(this, _imageList.at(nextSeq), _serial++, (DownloadConfig*)&ImageDownloader::DC_CACHE_ONLY);
    }
    
    _scheduler->performFunctionInCocosThread([this]{
        // schedule에 다음꺼를 등록한다.
        scheduleOnce(CC_SCHEDULE_SELECTOR(KenBurnsView::onNextTransition), (PAN_TIME-FADE_TIME));
    });
}

static float getAspectRatio(const cocos2d::Size& rect)
{
    return rect.width/rect.height;
}

static float truncate(float f, int d)
{
    float dShift = (float) std::pow(10, d);
    return ViewUtil::round(f*dShift) / dShift;
}

void KenBurnsView::pauseKenBurns()
{
    _runnable = false;
//    this->pauseSchedulerAndActions();
    pause();
}

void KenBurnsView::resumeKenBurns()
{
    _runnable = true;
    resume();
}

cocos2d::Rect KenBurnsView::generateRandomRect(const cocos2d::Size &imageSize)
{
    float ratio1 = getAspectRatio(imageSize);
    float ratio2 = getAspectRatio(_contentSize);
    
    cocos2d::Rect cropRect;
    float width, height;
    if (ratio1>ratio2) {
        width = (imageSize.height / _contentSize.height) * _contentSize.width;
        height = imageSize.height;
    } else {
        width = imageSize.width;
        height = (imageSize.width/_contentSize.width) * _contentSize.height;
    }
    cropRect.setRect(0, 0, width, height);
    
    float rnd = truncate(cocos2d::random(0.0f, 1.0f), 2);
    float factor = MINUM_RECT_FACTOR + ((1-MINUM_RECT_FACTOR)*rnd);
    
    width = factor * cropRect.size.width;
    height = factor * cropRect.size.height;

    float diffWidth = imageSize.width - width;
    float diffHeight = imageSize.height - height;
    float x = diffWidth>0.0f ? cocos2d::random(0.0f, diffWidth) : 0.0f;
    float y = diffHeight>0.0f ? cocos2d::random(0.0f, diffHeight) : 0.0f;
    
    return cocos2d::Rect(x, y, width, height);
}

