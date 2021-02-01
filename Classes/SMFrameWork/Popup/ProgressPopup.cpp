//
//  ProgressPopup.cpp
//  SMFrameWork
//
//  Created by SteveMac on 2018. 5. 24..
//

#include "ProgressPopup.h"
#include "PopupManager.h"
#include "../Base/ShaderNode.h"
#include "../UI/ProgressView.h"
#include "../Base/ViewAction.h"
#include "../Util/ViewUtil.h"

#define POPUP_ID_PROGRESS    (0xFFFFFE)
#define ACTION_TAG_SHOW  (0x100)

#define OPEN_TIME       (0.1)
#define CLOSE_TIME      (0.1)

#define BG_SIZE         (200.0f)
#define PROGRESS_SIZE (BG_SIZE*0.85f)

class ProgressPopup::ShowAction : public ViewAction::TransformAction {
public:
    CREATE_DELAY_ACTION(ShowAction);
    
    virtual void onStart() override {
        _popup = (ProgressPopup*)getTarget();
        _from = _popup->_showValue;
        _to = _isOpen ? 1 : 0;
    }
    
    virtual void onUpdate(float t) override {
        TransformAction::onUpdate(t);
        _popup->_showValue = ViewUtil::interpolation(_from, _to, t);
        _popup->setOpacity((GLubyte)(0xff*_popup->_showValue));
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
    
    ProgressPopup* _popup;
};

ProgressPopup* ProgressPopup::showProgress(float maxProgress, float delay)
{
    ProgressPopup * popup = nullptr;
    
    auto view = findPopupByTag(POPUP_ID_PROGRESS);
    if (view) {
        // 기존에 돌고 있냐?
        popup = reinterpret_cast<ProgressPopup*>(view);
        if (popup->getActionByTag(ACTION_TAG_SHOW)) {
            auto action = popup->getActionByTag(ACTION_TAG_SHOW);
            popup->stopAction(action);
        }
        
        PopupManager::getInstance().bringOnTop(view);
        
        float t = 1.0f - popup->_showValue;
        if (t>0) {
            auto action = ShowAction::create();
            action->setTag(ACTION_TAG_SHOW);
            action->setOpenValue(OPEN_TIME*t, 0);
            
            popup->runAction(action);
        }
    } else {
        popup = new (std::nothrow)ProgressPopup();
        if (popup && popup->initWithDelay(delay)) {
            popup->autorelease();
            popup->setTag(POPUP_ID_PROGRESS);
            
            auto action = ShowAction::create();
            action->setTag(ACTION_TAG_SHOW);
            action->setOpenValue(OPEN_TIME, delay);
            
            popup->runAction(action);
            
            auto sharedLayer = cocos2d::Director::getInstance()->getSharedLayer(cocos2d::Director::SharedLayer::POPUP);
            sharedLayer->addChild(popup);
        } else {
            CC_SAFE_DELETE(popup);
        }
    }
    
    return popup;
}

void ProgressPopup::close(bool success)
{
    if (success) {
        _progress->setOnCompleteCallback(CC_CALLBACK_1(ProgressPopup::onComplete, this));
        _progress->setProgress(100.0f);
    } else {
        dismiss(false);
    }
}

void ProgressPopup::onComplete(ProgressView *progress)
{
    auto popup = findPopupByTag(POPUP_ID_PROGRESS);
    if (popup) {
        popup->dismiss(true);
    }
    
    if (_dismissCallback) {
        _dismissCallback();
    }
}

ProgressPopup::ProgressPopup() : _showValue(0), _dismissCallback(nullptr)
{
    
}

ProgressPopup::~ProgressPopup()
{
    
}


bool ProgressPopup::initWithDelay(float delay)
{
    if (!Popup::init()) {
        return false;
    }
    
    auto s = cocos2d::Director::getInstance()->getWinSize();
    setContentSize(s);
    
    _progress = ProgressView::create(100.0f);
    _progress->setPosition(s/2);
    addChild(_progress);
    
    auto action = ViewAction::TransformAction::create();
    action->toAlpha(1).toScale(1).setTweenFunc(cocos2d::tweenfunc::TweenType::Back_EaseOut);
    action->setTimeValue(0.3f, delay);
    
    _progress->setOpacity(0);
    _progress->setScale(0.5f);
    _progress->runAction(action);
    
    return true;
}

void ProgressPopup::show()
{
    Popup::show();
}

void ProgressPopup::dismiss(bool imediate)
{
    if (imediate) {
        callbackOnDismiss();
        removeFromParent();
    } else {
        auto action = dynamic_cast<ShowAction*>(getActionByTag(ACTION_TAG_SHOW));
        if (action) {
            if (!action->isOpen()) {
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

int ProgressPopup::dispatchTouchEvent(const int action, const cocos2d::Touch *touch, const cocos2d::Vec2 *point, MotionEvent *event)
{
    return TOUCH_INTERCEPT;
}

void ProgressPopup::setProgress(const float progress)
{
    _progress->setProgress(progress);
}
