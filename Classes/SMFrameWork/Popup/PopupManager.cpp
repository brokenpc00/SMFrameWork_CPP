//
//  PopupManager.cpp
//  SMFrameWork
//
//  Created by SteveMac on 2018. 5. 24..
//

#include "PopupManager.h"
#include "Popup.h"
#include "../Base/SMView.h"
#include "../Base/ViewAction.h"

#define SHOW_TIME   (0.3)
#define HIDE_TIME   (0.3)


class PopupBg : public SMView
{
public:
    CREATE_VIEW(PopupBg);
    
    virtual int dispatchTouchEvent(const int action, const cocos2d::Touch* touch, const cocos2d::Vec2* point, MotionEvent* event) override {
        SMView::dispatchTouchEvent(action, touch, point, event);
        return TOUCH_INTERCEPT;
    }
    
    bool _isDismissed;
};

PopupManager& PopupManager::getInstance() {
    static PopupManager instance;
    return instance;
}

PopupManager::PopupManager()
{
    
}

PopupManager::~PopupManager()
{
    
}

void PopupManager::showPopup(Popup *targetPopup)
{
    if (targetPopup==nullptr) {
        return;
    }
    
    auto director = cocos2d::Director::getInstance();
    auto layer = director->getSharedLayer(cocos2d::Director::SharedLayer::POPUP);
    
    
    auto bg = PopupBg::create();
    bg->_isDismissed = false;
    bg->setTag(targetPopup->getTag());
    bg->setContentSize(director->getWinSize());
    bg->setBackgroundColor4F(cocos2d::Color4F(0, 0, 0, targetPopup->getBackgroundFadeOpacity()), SHOW_TIME);
    bg->addChild(targetPopup);
    
    layer->addChild(bg);
}

bool PopupManager::dismissPopup(Popup *targetPopup, bool imediate)
{
    if (targetPopup==nullptr) {
        return false;
    }
    
    auto director = cocos2d::Director::getInstance();
    auto layer = director->getSharedLayer(cocos2d::Director::SharedLayer::POPUP);
    
    auto children = layer->getChildren();
    for (auto child : children) {
        auto bg = dynamic_cast<PopupBg*>(child);
        if (bg) {
            auto bgChildren = bg->getChildren();
            for (auto bgChild : bgChildren) {
                auto popup = dynamic_cast<Popup*>(bgChild);
                if (popup && popup==targetPopup) {
                    if (bg->_isDismissed) {
                        // 이미 꺼진거??
                        return false;
                    }
                    if (imediate) {
                        bg->removeFromParent();
                    } else {
                        bg->_isDismissed = true;
                        bg->setBackgroundColor4F(cocos2d::Color4F(0, 0, 0, 0), HIDE_TIME);
                        
                        auto action = ViewAction::TransformAction::create();
                        action->removeOnFinish();
                        action->setTimeValue(HIDE_TIME+0.1, 0);
                        bg->runAction(action);
                    }
                    return true;
                }
            }
        }
    }
    
    return false;
}

Popup* PopupManager::findPopupByTag(const int tag)
{
    auto director = cocos2d::Director::getInstance();
    auto layer = director->getSharedLayer(cocos2d::Director::SharedLayer::POPUP);
    
    auto children = layer->getChildren();
    for (auto child : children) {
        auto bg = dynamic_cast<PopupBg*>(child);
        if (bg) {
            auto bgChildren = bg->getChildren();
            for (auto bgChild : bgChildren) {
                auto popup = dynamic_cast<Popup*>(bgChild);
                if (popup && popup->getTag()==tag && !bg->_isDismissed) {
                    // 찾은 넘이 있고 아직 안꺼졌으면 그넘 준다
                    return popup;
                }
            }
        } else {
            // bg에 있는게 아니면 바로 그넘이 popup
            auto popup = dynamic_cast<Popup*>(child);
            if (popup && popup->getTag()==tag) {
                return popup;
            }
        }
    }
    
    return nullptr;
}

void PopupManager::bringOnTop(Popup *popup)
{
    auto director = cocos2d::Director::getInstance();
    auto layer = director->getSharedLayer(cocos2d::Director::SharedLayer::POPUP);
    
    if (popup->getParent()!=layer) {
        return;
    }
    
    popup->retain();
    {
        // 떼었다가 붙인다.
        layer->removeChild(popup);
        layer->addChild(popup);
    }
    popup->release();
}

void PopupManager::dismissAllPopup()
{
    auto director = cocos2d::Director::getInstance();
    auto layer = director->getSharedLayer(cocos2d::Director::SharedLayer::POPUP);
    
    auto children = layer->getChildren();
    for (auto child : children) {
        auto bg = dynamic_cast<PopupBg*>(child);
        if (bg) {
            auto bgChildren = bg->getChildren();
            for (auto bgChild : bgChildren) {
                auto popup = dynamic_cast<Popup*>(bgChild);
                if (popup && !bg->_isDismissed) {
                    popup->dismiss();
                }
            }
        } else {
            auto popup = dynamic_cast<Popup*>(child);
            if (popup) {
                popup->dismiss();
            }
        }
    }
}


