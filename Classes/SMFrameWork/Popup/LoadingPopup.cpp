//
//  LoadingPopup.cpp
//  SMFrameWork
//
//  Created by SteveMac on 2018. 5. 24..
//

#include "LoadingPopup.h"
#include "../Base/ShaderNode.h"
#include "../UI/CircularProgress.h"
#include "../Base/ViewAction.h"
#include "../Const/SMFontColor.h"
#include "PopupManager.h"


#define ACTION_TAG_SHOW  (0x100)

#define OPEN_TIME       (0.1)
#define CLOSE_TIME      (0.1)

class LoadingPopup::ShowAction : public ViewAction::TransformAction
{
public:
    CREATE_DELAY_ACTION(ShowAction);
    
    virtual void onStart() override {
        _dialog = (LoadingPopup*)getTarget();
        _from = _dialog->_showValue;
        _to = _isOpen ? 1 : 0;
    }
    
    virtual void onUpdate(float t) override {
        TransformAction::onUpdate(t);
        
        _dialog->_showValue = ViewUtil::interpolation(_from, _to, t);
        _dialog->setOpacity((GLubyte)(0xff*_dialog->_showValue));
    }
    
    void setOpenValue(float duration, float delay) {
        _isOpen = true;
        setTimeValue(duration, delay);
    }
    
    void setCloseValue(float duration, float delay) {
        _isOpen = false;
        setTimeValue(duration, delay);
    }
    
    bool isOpen() {return _isOpen;}
    
private:
    bool _isOpen;
    float _from, _to;
    LoadingPopup* _dialog;
};


LoadingPopup* LoadingPopup::showLoading(float delay, int tag)
{
    LoadingPopup* loadingPopup = nullptr;
    
    auto popup = findPopupByTag(tag);
    if (popup) {
        // 이미 로딩중이다.
        loadingPopup = dynamic_cast<LoadingPopup*>(popup);
        
        // 일단 애니메이션 멈춤
        if (loadingPopup->getActionByTag(ACTION_TAG_SHOW)) {
            auto aciton = loadingPopup->getActionByTag(ACTION_TAG_SHOW);
            loadingPopup->stopAction(aciton);
        }
        
        
        // Top most로
        PopupManager::getInstance().bringOnTop(popup);
        
        // delay 없이 화면에 표시
        float t = 1.0 - loadingPopup->_showValue;
        if (t > 0) {
            auto action = ShowAction::create();
            action->setTag(ACTION_TAG_SHOW);
            action->setOpenValue(OPEN_TIME*t, 0);
            
            loadingPopup->runAction(action);
        }
    } else {
        // 로딩중이지 않음
        loadingPopup = new (std::nothrow) LoadingPopup();
        
        if (loadingPopup && loadingPopup->initWithDelay(delay)) {
            loadingPopup->autorelease();
            loadingPopup->setTag(tag);
            
            auto action = ShowAction::create();
            action->setTag(ACTION_TAG_SHOW);
            action->setOpenValue(OPEN_TIME, delay);

            loadingPopup->runAction(action);
            
            auto director = cocos2d::Director::getInstance();
            auto layer = director->getSharedLayer(cocos2d::Director::SharedLayer::POPUP);
            
            layer->addChild(loadingPopup);
        } else {
            CC_SAFE_DELETE(loadingPopup);
        }
    }
    
    return loadingPopup;
}

void LoadingPopup::close(bool imediate, int tag)
{
    auto popup = findPopupByTag(tag);
    if (popup) {
        popup->dismiss(imediate);
    }
}

bool LoadingPopup::isShow(int tag)
{
    auto popup = findPopupByTag(tag);
    if (popup) {
        return true;
    } else {
        return false;
    }
}

LoadingPopup::LoadingPopup()
{
    
}

LoadingPopup::~LoadingPopup()
{
    
}

#define BG_SIZE (200.0f)
#define PROGRESS_SIZE (BG_SIZE*0.85f)
#define PROGRESS_LINE_WIDTH 5.0f

bool LoadingPopup::initWithDelay(float delay)
{
    if (!Popup::init()) {
        return false;
    }
    
    auto s = _director->getWinSize();
    setContentSize(s);
    
    auto bg = ShapeSolidCircle::create();
    bg->setContentSize(cocos2d::Size(BG_SIZE, BG_SIZE));
    bg->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    bg->setPosition(s/2);
    bg->setColor4F(MAKE_COLOR4F(0, 0.9));
    addChild(bg);
    
    auto progress = CircularProgress::createIndeterminate();
    progress->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    progress->setContentSize(cocos2d::Size(PROGRESS_SIZE, PROGRESS_SIZE));
    progress->setLineWidth(PROGRESS_LINE_WIDTH);
    progress->setPosition(bg->getContentSize()/2);
    progress->setColor(cocos2d::Color3B::WHITE);
    bg->addChild(progress);
    
    auto action = ViewAction::TransformAction::create();
    action->toScale(1).setTweenFunc(cocos2d::tweenfunc::TweenType::Back_EaseOut);
    action->setTimeValue(0.3, delay);
    
    bg->setScale(0.5);
    bg->runAction(action);
    
    return true;
}

void LoadingPopup::show()
{
    Popup::show();
}

void LoadingPopup::dismiss(bool imediate)
{
    if (imediate) {
        callbackOnDismiss();
        removeFromParent();
    } else {
        auto action = dynamic_cast<ShowAction*>(getActionByTag(ACTION_TAG_SHOW));
        if (action) {
            if (!action->isOpen()) {
                // 종료중이니? 그럼 말고
                return;
            }
            
            stopAction(action);
        }
        
        action = ShowAction::create();
        action->setTag(ACTION_TAG_SHOW);
        action->setCloseValue(CLOSE_TIME, 0);
        action->removeOnFinish();
        
        runAction(action);
        callbackOnDismiss();
    }
}

int LoadingPopup::dispatchTouchEvent(const int action, const cocos2d::Touch *touch, const cocos2d::Vec2 *point, MotionEvent *event)
{
    return TOUCH_INTERCEPT;
}

