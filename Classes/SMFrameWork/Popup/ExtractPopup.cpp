//
//  ExtractPopup.cpp
//  SMFrameWork
//
//  Created by SteveMac on 2018. 9. 13..
//

#include "ExtractPopup.h"
#include "../Base/ShaderNode.h"
#include "../Const/SMFontColor.h"
#include "../Base/ViewAction.h"
#include "PopupManager.h"
#include "../Util/StringUtil.h"

#define POPUP_ID_EXTRACT    (0xFFFFF6)
#define ACTION_TAG_SHOW  (0x100)

#define OPEN_TIME       (0.1)
#define CLOSE_TIME      (0.1)


class ExtractPopup::ShowAction : public ViewAction::TransformAction
{
public:
    CREATE_DELAY_ACTION(ShowAction);
    
    virtual void onStart() override {
        _dialog = (ExtractPopup*)getTarget();
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
    ExtractPopup* _dialog;
};


ExtractPopup* ExtractPopup::extractShow(float delay)
{
    ExtractPopup* extractPopup = nullptr;
    
    auto popup = findPopupByTag(POPUP_ID_EXTRACT);
    if (popup) {
        // 이미 로딩중이다.
        extractPopup = dynamic_cast<ExtractPopup*>(popup);
        
        // 일단 애니메이션 멈춤
        if (extractPopup->getActionByTag(ACTION_TAG_SHOW)) {
            auto aciton = extractPopup->getActionByTag(ACTION_TAG_SHOW);
            extractPopup->stopAction(aciton);
        }
        
        
        // Top most로
        PopupManager::getInstance().bringOnTop(popup);
        
        // delay 없이 화면에 표시
        float t = 1.0 - extractPopup->_showValue;
        if (t > 0) {
            auto action = ShowAction::create();
            action->setTag(ACTION_TAG_SHOW);
            action->setOpenValue(OPEN_TIME*t, 0);
            
            extractPopup->runAction(action);
        }
    } else {
        // 로딩중이지 않음
        extractPopup = new (std::nothrow) ExtractPopup();
        
        if (extractPopup && extractPopup->initWithDelay(delay)) {
            extractPopup->autorelease();
            extractPopup->setTag(POPUP_ID_EXTRACT);
            
            auto action = ShowAction::create();
            action->setTag(ACTION_TAG_SHOW);
            action->setOpenValue(OPEN_TIME, delay);
            
            extractPopup->runAction(action);
            
            auto director = cocos2d::Director::getInstance();
            auto layer = director->getSharedLayer(cocos2d::Director::SharedLayer::POPUP);
            
            layer->addChild(extractPopup);
        } else {
            CC_SAFE_DELETE(extractPopup);
        }
    }
    
    return extractPopup;
}

void ExtractPopup::close(bool imediate)
{
    auto popup = findPopupByTag(POPUP_ID_EXTRACT);
    if (popup) {
        popup->dismiss(imediate);
    }
    popup = nullptr;
}

bool ExtractPopup::isShow()
{
    auto popup = findPopupByTag(POPUP_ID_EXTRACT);
    if (popup) {
        return true;
    } else {
        return false;
    }
}

ExtractPopup::ExtractPopup()
{
    
}

ExtractPopup::~ExtractPopup()
{
    
}

#define BG_SIZE (600.0f)
#define PROGRESS_SIZE (BG_SIZE*0.85f)
#define PROGRESS_LINE_WIDTH 5.0f

bool ExtractPopup::initWithDelay(float delay)
{
    if (!Popup::init()) {
        return false;
    }
    
    auto s = _director->getWinSize();
    setContentSize(s);

    auto bg = ShapeSolidRoundRect::create();
    bg->setContentSize(cocos2d::Size(BG_SIZE, BG_SIZE));
    bg->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    bg->setPosition(s/2);
    bg->setColor4F(MAKE_COLOR4F(0, 0.9));
    bg->setCornerRadius(60);
    addChild(bg);
    
    _progress = ShapeRoundLine::create();
    _progress->setLineWidth(ShapeNode::DEFAULT_ANTI_ALIAS_WIDTH*4.0f);
    _progress->setColor4F(MAKE_COLOR4F(0xffffff, 1.0f));
    _progress->setAnchorPoint(cocos2d::Vec2::ZERO);
    _progress->setPosition(cocos2d::Vec2(40, 80));
    _progress->setContentSize(cocos2d::Size::ZERO);
    bg->addChild(_progress);

    _title = cocos2d::Label::createWithTTF("", SMFontConst::NotoSansMedium, 45);
    _title->setTextColor(MAKE_COLOR4B(0xffffffff));
    _title->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    _title->setPosition(cocos2d::Vec2(BG_SIZE/2, BG_SIZE*0.82));
    bg->addChild(_title);
    
    _desc = cocos2d::Label::createWithTTF("", SMFontConst::NotoSansLight, 40);
    _desc->setTextColor(MAKE_COLOR4B(0xffffffff));
    _desc->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    _desc->setPosition(bg->getContentSize()/2);
    bg->addChild(_desc);
    
    auto action = ViewAction::TransformAction::create();
    action->toScale(1).setTweenFunc(cocos2d::tweenfunc::TweenType::Back_EaseOut);
    action->setTimeValue(0.3, delay);
    
    bg->setScale(0.5);
    bg->runAction(action);
    
    return true;
}

void ExtractPopup::show()
{
    Popup::show();
}

void ExtractPopup::dismiss(bool imediate)
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

int ExtractPopup::dispatchTouchEvent(const int action, const cocos2d::Touch *touch, const cocos2d::Vec2 *point, MotionEvent *event)
{
    return TOUCH_INTERCEPT;
}

void ExtractPopup::setProgress(int cur, int total, float progress, std::string desc)
{

    auto popup = findPopupByTag(POPUP_ID_EXTRACT);
    if (popup) {
        auto extract = dynamic_cast<ExtractPopup*>(popup);
        if (extract) {
            auto s = cocos2d::Director::getInstance()->getWinSize();
//            std::vector<std::string> sp = StringUtil::separateStringByString(desc, "/");
//            desc = *(sp.end());
            std::string memo = "";
            if (total>0) {
                if (total==-1) {
                    // 파싱
                    extract->_title->setString("Parsing ePub...");
                    memo = cocos2d::StringUtils::format("%s", desc.c_str());

                } else {
                    // 파일
                    extract->_cur = cur+1;
                    extract->_total = total;
                    
                    float width = (BG_SIZE-80) * progress;
                    extract->_progress->setContentSize(cocos2d::Size(width, 20));
                    extract->_title->setString("Extract files...");
                    memo = cocos2d::StringUtils::format("%s", desc.c_str());
                }
            } else {
                // 폴더
                extract->_title->setString("Make folders...");
                memo = cocos2d::StringUtils::format("%s", desc.c_str());
            }
            
            extract->_desc->setString(memo);
        }
        
    }
    
}
