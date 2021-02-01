//
//  ProgressView.cpp
//  SMFrameWork
//
//  Created by SteveMac on 2018. 6. 1..
//

#include "ProgressView.h"
#include "../UI/CircularProgress.h"
#include "../Base/ShaderNode.h"
#include "../Util/ViewUtil.h"
#include "../Base/ViewAction.h"
//#include "../Const/SMFontColor.h"


#define PANNEL_WIDTH (206.0f)
#define LINE_WIDTH (8.0f)

#define ENTER_DURATION (0.35f)

#define RING_EXPAND_DURATION (0.2)
#define CHECK_DURATION (0.15)
#define EXIT_DURATION   (0.15)

static const cocos2d::Color4F BG_COLOR = MAKE_COLOR4F(0x000000, 0.7f);

class ProgressView::CheckAction : public cocos2d::ActionInterval {
public:
    static CheckAction * create(float duration) {
        auto action = new CheckAction();
        action->initWithDuration(duration);
        action->autorelease();
        
        return action;
    }
    
    virtual void startWithTarget(cocos2d::Node* target) override {
        cocos2d::ActionInterval::startWithTarget(target);
        auto view = static_cast<ProgressView*>(_target);
        _line1 = view->_checkLine1;
        _line2 = view->_checkLine2;
        
        _line1->setVisible(false);
        _line2->setVisible(false);
    }
    
    virtual void update(float t) override {
        // 시계 방향으로 넘어가는 시간동안
        t = 1.0f - std::cos(t*M_PI_2);
        
        const float phaseDiv = 0.5;
        
        if (t<phaseDiv) {
            // 반 넘어가기 전이면 1번 라인 그리고
            _line1->setVisible(true);
            
            t = t / phaseDiv;
            // 대각선으로 내려와
            t = std::sin(t * M_PI_2);
            _line1->setLengthScale(t);
        } else {
            // 반 넘어간 후는 2번 라인 그리자
            _line2->setVisible(true);
            
            t = (t-phaseDiv) / (1.0f-phaseDiv);
            // 대각선으로 올라감
            t = 1.0f - std::cos(t * M_PI_2);
            _line2->setLengthScale(t);
        }
    }
    
private:
    ShapeRoundLine* _line1;
    ShapeRoundLine* _line2;
};

// progress 100% 끝나면 좀 커지는 액션...
class ProgressView::RingExpandAction : public cocos2d::ActionInterval {
public:
    static RingExpandAction * create(float duration) {
        auto action = new RingExpandAction();
        action->initWithDuration(duration);
        action->autorelease();
        
        return action;
    }
    
    virtual void startWithTarget(cocos2d::Node* target) override {
        cocos2d::ActionInterval::startWithTarget(target);
        auto view = static_cast<ProgressView*>(_target);
        _progress = view->_progress;
        _ring = view->_ring;
        _bgCircle = view->_bgCircle;
        
        _progress->setVisible(false);
        _ring->setVisible(true);
    }
    
    virtual void update(float t) override {
        t = 1.0f - std::cos(t * M_PI_2);
        
        float outerSize = std::min(PANNEL_WIDTH, ViewUtil::interpolation(PANNEL_WIDTH-32, PANNEL_WIDTH, t*1.5));
        float diff = (outerSize - (PANNEL_WIDTH-32)) / 2;
        float lineSize = std::min(PANNEL_WIDTH/2, diff+ViewUtil::interpolation(LINE_WIDTH, PANNEL_WIDTH/2, t));
        _ring->setContentSize(cocos2d::Size(outerSize, outerSize));
        _ring->setLineWidth(lineSize);
        
        float opacity = ViewUtil::interpolation(BG_COLOR.a, 0.5, t);
        _bgCircle->setOpacity(0xff*opacity);
        
        if (t >= 1.0f) {
            _bgCircle->setColor4F(MAKE_COLOR4F(0x222222, 1.0f));
            _ring->setVisible(false);
        }
    }
    
private:
    CircularProgress* _progress;
    ShapeCircle* _ring;
    ShapeSolidCircle* _bgCircle;
};

void ProgressView::hide()
{
    stopAllActions();
    
    reset();
    
    setVisible(false);
    
    removeAllChildren();
    
    removeFromParent();
}

ProgressView* ProgressView::create(float maxProgress)
{
    auto progressView = new(std::nothrow) ProgressView();
    if (progressView) {
        if (progressView->init()) {

            progressView->setMaxProgress(maxProgress);
            progressView->autorelease();
        } else {
            CC_SAFE_DELETE(progressView);
        }
    }
    return progressView;
}

ProgressView* ProgressView::show(cocos2d::Node *parent
                                 , float maxProgress
                                 , std::function<void(ProgressView* sender)> completeCallback
                                 , std::function<void(ProgressView* sender, float currentProgress, float maxProgress)> progressCallback) {
    auto progressView = new(std::nothrow) ProgressView();
    if (progressView) {
        if (progressView->init()) {
            progressView->setMaxProgress(maxProgress);
            progressView->setOnCompleteCallback(completeCallback);
            progressView->setOnProgressUpdateCallback(progressCallback);
            
            parent->addChild(progressView);
            
            auto s = cocos2d::Director::getInstance()->getWinSize();
            progressView->setPosition(s/2);
            
            progressView->startShowAction();
        } else {
            CC_SAFE_DELETE(progressView);
            progressView = nullptr;
        }
    }
    
    return progressView;
}


ProgressView::ProgressView() : _progress(nullptr)
, _onCompleteCallback(nullptr)
, _onProgressUpdateCallback(nullptr)
, _withoutDonAction(false)
{
    
}

ProgressView::~ProgressView()
{
    
}

bool ProgressView::init()
{
    if (!SMView::init()) {
        return false;
    }
    
    setContentSize(cocos2d::Size(PANNEL_WIDTH, PANNEL_WIDTH));
    setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    
    // 배경 원
    _bgCircle = ShapeSolidCircle::create();
    _bgCircle->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    _bgCircle->setPosition(PANNEL_WIDTH/2, PANNEL_WIDTH/2);
    addChild(_bgCircle);
    
    // progress
    _ring = ShapeCircle::create();
    _ring->setPosition(PANNEL_WIDTH/2, PANNEL_WIDTH/2);
    _ring->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    addChild(_ring);
    
    _progress = CircularProgress::createDeterminate();
    _progress->setPosition(PANNEL_WIDTH/2, PANNEL_WIDTH/2);
    _progress->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    addChild(_progress);
    
    // 끝날때 체크 라인
    _checkLine1 = ShapeRoundLine::create();
    _checkLine1->setLineWidth(LINE_WIDTH);
    _checkLine1->line(64, PANNEL_WIDTH-102, 91, PANNEL_WIDTH-128);
    _checkLine1->setLengthScale(0.95);
    addChild(_checkLine1);
    
    _checkLine2 = ShapeRoundLine::create();
    _checkLine2->setLineWidth(LINE_WIDTH);
    _checkLine2->line(91, PANNEL_WIDTH-128, 142, PANNEL_WIDTH-78);
    addChild(_checkLine2);
    
    _progress->setMaxProgress(100.0f);
    _progress->start();
    _progress->setLineWidth(LINE_WIDTH);
    
    _progress->_onProgressUpdateCallback = CC_CALLBACK_3(ProgressView::onProgressUpdate, this);
    
    reset();
    
    setVisible(false);
    
    return true;
}

void ProgressView::reset()
{
    _bgCircle->setContentSize(_contentSize);
    _bgCircle->setColor4F(BG_COLOR);
    _bgCircle->setVisible(true);
    
    _ring->setColor4F(MAKE_COLOR4F(0xffffff, 1.0f));
    _ring->setContentSize(cocos2d::Size(PANNEL_WIDTH-32, PANNEL_WIDTH-32));
    _ring->setLineWidth(LINE_WIDTH);
    _ring->setVisible(false);
    
    _progress->setContentSize(cocos2d::Size(PANNEL_WIDTH-32, PANNEL_WIDTH-32));
    _progress->setColor(MAKE_COLOR3B(0xffffff));
    _progress->setLineWidth(LINE_WIDTH);
    _progress->setVisible(true);
    
    _checkLine1->setVisible(false);
    _checkLine2->setVisible(false);
}

void ProgressView::setOnProgressUpdateCallback(std::function<void (ProgressView * sender, float currentProgress, float maxProgress)> callback)
{
    _onProgressUpdateCallback = callback;
}

void ProgressView::setOnCompleteCallback(std::function<void (ProgressView * sender)> callback)
{
    _onCompleteCallback = callback;
}

void ProgressView::onProgressUpdate(CircularProgress *sender, float currentProgress, float maxProgress)
{
    if (_onProgressUpdateCallback) {
        _onProgressUpdateCallback(this, currentProgress, maxProgress);
    }
    
    if (currentProgress == maxProgress) {
        _onProgressUpdateCallback = nullptr;
        startDoneAction();
    }
}

void ProgressView::startShowAction()
{
    stopAllActions();
    
    reset();
    
    setVisible(true);
    
    setOpacity(0);
    setScale(0.5f);
    
    auto enter = cocos2d::Spawn::create(cocos2d::EaseBackOut::create(cocos2d::ScaleTo::create(ENTER_DURATION, 1.0f)),
                                        cocos2d::FadeTo::create(ENTER_DURATION, 0xff),
                                        NULL);
    
    runAction(enter);
}

// progress 끝나고 check 라인 그리기
void ProgressView::startDoneAction()
{
    reset();
    
    setOpacity(0xff);
    setScale(1.0f);
    
    _progress->setProgress(_progress->getMaxProgress());
    
    if (_withoutDonAction) {
        return;
    }
    
    // 색변하고
    auto colorTo = cocos2d::EaseIn::create(ViewAction::ColorTo::create(RING_EXPAND_DURATION*0.7f, MAKE_COLOR4F(0x222222, 1.0f)), 0.8f);
    
    // 커지고
    auto expandTo = RingExpandAction::create(RING_EXPAND_DURATION);
    
    // 체크하고
    auto check = cocos2d::Spawn::create(CheckAction::create(CHECK_DURATION),
                                         cocos2d::Sequence::create(cocos2d::DelayTime::create(0.15f), cocos2d::EaseInOut::create(ViewAction::ScaleSine::create(0.4f, 1.1f), 1.0f),
                                                                   NULL),
                                         NULL);
    
    // 종료한다
    auto exit = cocos2d::Spawn::create(cocos2d::EaseIn::create(cocos2d::ScaleTo::create(EXIT_DURATION, 0.7f), 1.5f),
                                       cocos2d::FadeTo::create(EXIT_DURATION, 0),
                                       NULL);
    
    auto sequence = cocos2d::Sequence::create(expandTo,
                                              cocos2d::DelayTime::create(0.2f),
                                              check,
                                              cocos2d::DelayTime::create(1.5f),
                                              exit,
                                              cocos2d::CallFuncN::create([this](cocos2d::Node* target){
                                                                            if (_onCompleteCallback) {
                                                                                _onCompleteCallback(this);
                                                                                target->removeFromParent();
                                                                                target = nullptr;
                                                                            }
                                                                        }),
                                              NULL);
    
    auto action = cocos2d::Spawn::create(colorTo, sequence, NULL);
    
    runAction(action);
}
