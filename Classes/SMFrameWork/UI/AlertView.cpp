//
//  AlertView.cpp
//  iPet
//
//  Created by KimSteve on 2017. 6. 23..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#include "AlertView.h"
#include "../Base/SMButton.h"
#include "../Base/SMImageView.h"
#include "../Base/SMScene.h"
#include "../Base/ShaderUtil.h"
#include "../Const/SMFontColor.h"
#include "../Const/SMViewConstValue.h"
#include "../Util/ViewUtil.h"

enum ErrorCode {
    ErrorCode_Success = 0,
    ErrorCode_EmptyInputBox = 301,
    ErrorCode_NoImage,
    ErrorCode_NoSign,
    ErrorCode_NoRQBar,
    ErrorCode_InvalidPhoneNumber,
    ErrorCode_InvalidGPS,
};

static AlertView* _instance = nullptr;

#define ALERT_VIEW_ID 0xff0033

#define kMessageBgWidth (cocos2d::Director::getInstance()->getWinSize().width-80)
#define kMessageBgHeight 600
#define kButtonHeight 140

#define TOP_MENU_HEIGHT (SMViewConstValue::Size::TOP_MENU_HEIGHT+SMViewConstValue::Size::getStatusHeight())
#define TOP_NOTI_HEIGHT 336


AlertView::AlertView() :
_isAlreadyShow(false)
, _duration(ALERT_DURATION)
, _alertSprite(nullptr)
, _notiSprite(nullptr)
, _alertImageView(nullptr)
, _notiImageView(nullptr)
, _alertMemoLabel(nullptr)
, _messageBodyBgView(nullptr)
, _okButton(nullptr)
, _cancelButton(nullptr)
, _closeButton(nullptr)
, _titleLabel(nullptr)
, _contentLabelBgView(nullptr)
, _contentLabel(nullptr)
, _okBlock(nullptr)
, _cancelBlock(nullptr)
{
    
}

AlertView::~AlertView()
{
    
}

AlertView * AlertView::getInstance()
{
    auto s = cocos2d::Director::getInstance()->getWinSize();
    if (_instance==nullptr) {
        _instance = AlertView::create(0, s.height, s.width, TOP_MENU_HEIGHT);
        _instance->setBackgroundColor4F(MAKE_COLOR4F(0xff3a2f, 1.0f));
        _instance->setLocalZOrder(90);
        _instance->retain();
        _instance->_isAlreadyShow = false;
        
        _instance->_alertMemoLabel = cocos2d::Label::createWithSystemFont("", SMFontConst::SystemFontRegular, 48);
        _instance->_alertMemoLabel->setTextColor(cocos2d::Color4B::WHITE);
        _instance->_alertMemoLabel->setAnchorPoint(cocos2d::Vec2::ZERO);
        _instance->addChild(_instance->_alertMemoLabel);
        _instance->_duration = ALERT_DURATION;
        
        _instance->_messageBodyBgView = SMView::create(0, s.width/2-kMessageBgWidth/2, s.height/2-kMessageBgHeight/2, kMessageBgWidth, kMessageBgHeight);
        _instance->_messageBodyBgView->setBackgroundColor4F(cocos2d::Color4F::WHITE);
        _instance->addChild(_instance->_messageBodyBgView);
        _instance->_messageBodyBgView->setVisible(false);
        
        _instance->_cancelButton = SMButton::create(0, SMButton::Style::SOLID_RECT, 0, 0, kMessageBgWidth/2, kButtonHeight);
        _instance->_cancelButton->setTextSystemFont("", SMFontConst::SystemFontRegular, 44);
        _instance->_cancelButton->setTextColor(SMButton::State::NORMAL, cocos2d::Color4F::WHITE);
        _instance->_cancelButton->setButtonColor(SMButton::State::NORMAL, MAKE_COLOR4F(0xadafb3, 1.0f));
        _instance->_cancelButton->setButtonColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x9a9caf, 1.0f));
        _instance->_messageBodyBgView->addChild(_instance->_cancelButton);
        _instance->_cancelButton->setOnClickListener(_instance);
        
        _instance->_okButton = SMButton::create(0, SMButton::Style::SOLID_RECT, kMessageBgWidth/2, 0, kMessageBgWidth/2, kButtonHeight);
        _instance->_okButton->setTextSystemFont("", SMFontConst::SystemFontRegular, 44);
        _instance->_okButton->setTextColor(SMButton::State::NORMAL, cocos2d::Color4F::WHITE);
        _instance->_okButton->setButtonColor(SMButton::State::NORMAL, MAKE_COLOR4F(0xff3a2f, 1.0f));
        _instance->_okButton->setButtonColor(SMButton::State::PRESSED, MAKE_COLOR4F(0xd53128, 1.0f));
        _instance->_messageBodyBgView->addChild(_instance->_okButton);
        _instance->_okButton->setOnClickListener(_instance);
        
        _instance->_titleLabel = cocos2d::Label::createWithSystemFont("", SMFontConst::SystemFontBold, 50);
        _instance->_titleLabel->setAnchorPoint(cocos2d::Vec2::ZERO);
        _instance->_titleLabel->setTextColor(SMColorConst::COLOR_4B_222222);
        _instance->_titleLabel->setVisible(false);
        _instance->_messageBodyBgView->addChild(_instance->_titleLabel);
        
        _instance->_contentLabelBgView = SMView::create(0, 0, kButtonHeight, _instance->_messageBodyBgView->getContentSize().width, _instance->_messageBodyBgView->getContentSize().height-kButtonHeight);
        _instance->_messageBodyBgView->addChild(_instance->_contentLabelBgView);
        
        _instance->_contentLabel = cocos2d::Label::createWithSystemFont("", SMFontConst::SystemFontRegular, 50);
        _instance->_contentLabel->setAnchorPoint(cocos2d::Vec2::ZERO);
        _instance->_contentLabel->setAlignment(cocos2d::TextHAlignment::CENTER);
        _instance->_contentLabel->setTextColor(SMColorConst::COLOR_4B_222222);
        _instance->_contentLabelBgView->addChild(_instance->_contentLabel);
        
        _instance->setOnClickListener(_instance);
    }
    
    return _instance;
}

void AlertView::hideAlert()
{
    _instance->removeFromParent();
}

void AlertView::closeAlertView()
{
    _instance->removeFromParent();
}

void AlertView::showToast(std::string title, std::string toastMessage, std::string cancelTitle, std::string okTitle, const OK_BLOCK &okBlock, const CANCEL_BLOCK &cancelBlock)
{
    AlertView::getInstance()->_alertType = kAlertTypeToast;
    AlertView::getInstance()->_okBlock = okBlock;
    AlertView::getInstance()->_cancelBlock = cancelBlock;
    auto s = cocos2d::Director::getInstance()->getWinSize();
    AlertView::getInstance()->_alertMemoLabel->setVisible(false);
    AlertView::getInstance()->_messageBodyBgView->setVisible(true);
    AlertView::getInstance()->_okButton->setVisible(true);
    AlertView::getInstance()->_cancelButton->setVisible(true);
    AlertView::getInstance()->_okButton->setButtonColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x494949, 1.0f));
    AlertView::getInstance()->_okButton->setButtonColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x373737, 1.0f));
    AlertView::getInstance()->_contentLabel->setVisible(true);
    AlertView::getInstance()->setBackgroundColor4F(cocos2d::Color4F(0, 0, 0, 0));
    
    AlertView::getInstance()->setContentSize(s);
    AlertView::getInstance()->setPosition(cocos2d::Vec2::ZERO);
    AlertView::getInstance()->_contentLabel->setString(toastMessage);
    
    auto tmpLabel = cocos2d::Label::createWithSystemFont(toastMessage, SMFontConst::SystemFontRegular, 50);
    if (tmpLabel->getContentSize().width>kMessageBgWidth-180) {
        AlertView::getInstance()->_contentLabel->setDimensions(kMessageBgWidth-180, 0);
    } else {
        AlertView::getInstance()->_contentLabel->setDimensions(0, 0);
    }
    
    AlertView::getInstance()->_okButton->setText(okTitle);
    AlertView::getInstance()->_cancelButton->setText(cancelTitle);
    AlertView::getInstance()->_contentLabel->setPosition(cocos2d::Vec2(AlertView::getInstance()->_messageBodyBgView->getContentSize().width/2-AlertView::getInstance()->_contentLabel->getContentSize().width/2, AlertView::getInstance()->_messageBodyBgView->getContentSize().height/2-AlertView::getInstance()->_contentLabel->getContentSize().height/2));
    
    SMView * layer = (SMView*)cocos2d::Director::getInstance()->getSharedLayer(cocos2d::Director::SharedLayer::POPUP);
    _instance->setTag(ALERT_VIEW_ID);
    layer->addChild(AlertView::getInstance());
    AlertView::getInstance()->setBackgroundColor4F(cocos2d::Color4F(0, 0, 0, 0.6f), 0.3f);
}

void AlertView::showConfirm(std::string title, std::string confirmMessage, std::string cancelTitle, std::string okTitle, const OK_BLOCK &okBlock, const CANCEL_BLOCK &cancelBlock)
{
    AlertView::getInstance()->_alertType = kAlertTypeConfirm;
    AlertView::getInstance()->_okBlock = okBlock;
    AlertView::getInstance()->_cancelBlock = cancelBlock;
    auto s = cocos2d::Director::getInstance()->getWinSize();
    AlertView::getInstance()->_alertMemoLabel->setVisible(false);
    AlertView::getInstance()->_messageBodyBgView->setVisible(true);
    AlertView::getInstance()->_okButton->setVisible(true);
    AlertView::getInstance()->_cancelButton->setVisible(true);
    AlertView::getInstance()->_okButton->setButtonColor(SMButton::State::NORMAL, MAKE_COLOR4F(0xff322f, 1.0f));
    AlertView::getInstance()->_okButton->setButtonColor(SMButton::State::PRESSED, MAKE_COLOR4F(0xd53128, 1.0f));

    AlertView::getInstance()->_contentLabel->setVisible(true);
    AlertView::getInstance()->setBackgroundColor4F(cocos2d::Color4F(0, 0, 0, 0));
    
    AlertView::getInstance()->setContentSize(s);
    AlertView::getInstance()->setPosition(cocos2d::Vec2::ZERO);
    AlertView::getInstance()->_contentLabel->setString(confirmMessage);
    
    auto tmplabel = cocos2d::Label::createWithSystemFont(confirmMessage, SMFontConst::SystemFontRegular, 50);
    if (tmplabel->getContentSize().width>kMessageBgWidth-180) {
        AlertView::getInstance()->_contentLabel->setDimensions(kMessageBgWidth-180, 0);
    } else {
        AlertView::getInstance()->_contentLabel->setDimensions(0, 0);
    }
    
    AlertView::getInstance()->_okButton->setText(okTitle);
    AlertView::getInstance()->_cancelButton->setText(cancelTitle);
    AlertView::getInstance()->_contentLabel->setPosition(cocos2d::Vec2(AlertView::getInstance()->_messageBodyBgView->getContentSize().width/2-AlertView::getInstance()->_contentLabel->getContentSize().width/2, AlertView::getInstance()->_messageBodyBgView->getContentSize().height/2-AlertView::getInstance()->_contentLabel->getContentSize().height/2));
    
    SMView * layer = (SMView*)cocos2d::Director::getInstance()->getSharedLayer(cocos2d::Director::SharedLayer::POPUP);
    _instance->setTag(ALERT_VIEW_ID);
    layer->addChild(AlertView::getInstance());
    
    AlertView::getInstance()->setBackgroundColor4F(MAKE_COLOR4F(0x0000, 0.6f), 0.3f);
}


void AlertView::onClick(SMView *view)
{
    if (view==_cancelButton) {
        cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread([&, this]{
            AlertView::getInstance()->closeAlertView();
        });
        if (AlertView::getInstance()->_cancelBlock) {
            AlertView::getInstance()->_cancelBlock();
        }
    } else if (view==_okButton) {
        cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread([&, this]{
            AlertView::getInstance()->closeAlertView();
        });
        if (AlertView::getInstance()->_okBlock) {
            AlertView::getInstance()->_okBlock();
        }
    }
}

void AlertView::showInformation(std::string infoMessage, float duration)
{
    AlertView::getInstance()->_alertType = kAlertTypeInformation;
    
    auto s = cocos2d::Director::getInstance()->getWinSize();
    
    AlertView::getInstance()->_duration = duration;
    AlertView::getInstance()->_alertMemoLabel->setVisible(true);
    AlertView::getInstance()->_messageBodyBgView->setVisible(false);
    AlertView::getInstance()->_okButton->setVisible(false);
    AlertView::getInstance()->_cancelButton->setVisible(false);
    AlertView::getInstance()->_titleLabel->setVisible(false);
    AlertView::getInstance()->_contentLabel->setVisible(false);
    
    AlertView::getInstance()->setPosition(cocos2d::Vec2(0, s.height));
    AlertView::getInstance()->setContentSize(cocos2d::Size(s.width, TOP_MENU_HEIGHT/2));
    
    if (AlertView::getInstance()->_isAlreadyShow) {
        AlertView::getInstance()->setContentSize(cocos2d::Size(s.width, TOP_MENU_HEIGHT/2));
        AlertView::getInstance()->setBackgroundColor4F(MAKE_COLOR4F(0x00a133, 1.0f));
        AlertView::getInstance()->_alertMemoLabel->setString(infoMessage);
        AlertView::getInstance()->_alertMemoLabel->setPosition(cocos2d::Vec2(s.width/2-AlertView::getInstance()->_alertMemoLabel->getContentSize().width/2, TOP_MENU_HEIGHT/2-AlertView::getInstance()->_alertMemoLabel->getContentSize().height/2));
        
        if (AlertView::getInstance()->isScheduled(schedule_selector(AlertView::setHideAlert))) {
            AlertView::getInstance()->unschedule(schedule_selector(AlertView::setHideAlert));
        }
    } else {
        AlertView::getInstance()->setPosition(cocos2d::Vec2(0, s.height));
        AlertView::getInstance()->setContentSize(cocos2d::Size(s.width, TOP_NOTI_HEIGHT/2));
        AlertView::getInstance()->setBackgroundColor4F(MAKE_COLOR4F(0x00a133, 1.0f));
        
        AlertView::getInstance()->_isAlreadyShow = true;
        SMView * layer = (SMView*)cocos2d::Director::getInstance()->getSharedLayer(cocos2d::Director::SharedLayer::POPUP);
        _instance->setTag(ALERT_VIEW_ID);
        layer->addChild(_instance);
        
        AlertView::getInstance()->_alertMemoLabel->setString(infoMessage);
        AlertView::getInstance()->_alertMemoLabel->setPosition(cocos2d::Vec2(s.width/2-AlertView::getInstance()->_alertMemoLabel->getContentSize().width/2, TOP_MENU_HEIGHT/2-AlertView::getInstance()->_alertMemoLabel->getContentSize().height/2));
        
        auto moveDown = cocos2d::MoveTo::create(ALERT_MOVE_DOWN, cocos2d::Vec2(0, s.width-TOP_MENU_HEIGHT/2));
        AlertView::getInstance()->runAction(moveDown);
    }
    
    AlertView::getInstance()->schedule(schedule_selector(AlertView::setHideAlert), AlertView::getInstance()->_duration);
}

std::string AlertView::getErrorMessageFromCode(int alertCode)
{
    std::string message = "";

    // 나중에 Error Code에 맞는 string을 내려줘야 함.
    return message;
}

void AlertView::showAlert(int alertCode, std::string msg, float duration)
{
    std::string errMsg = "";
    // 서버 메시지 인지... 내부 메시지인지 구분
    if (alertCode!=9999) {
        // 내부 메시지라면 변환해서 내려줌
        errMsg = getErrorMessageFromCode(alertCode);
    } else {
        // 서버 메시지면 그냥 내려줌
        errMsg = msg;
    }
    AlertView::showAlert(errMsg, duration);
    // 로그아웃 코드라면... (여기서는 그냥 -99라고 했음)... 로그 아웃처리한다.
    if (alertCode==-99) {
        // 지금은 없으니까 그냥 패스
    }
}

bool AlertView::isAlreadyShow()
{
    SMView * layer = (SMView*)cocos2d::Director::getInstance()->getSharedLayer(cocos2d::Director::SharedLayer::POPUP);
    
    for (auto child : layer->getChildren()) {
        if (child->getTag()==ALERT_VIEW_ID) {
            return true;
        }
    }

    return false;
}

void AlertView::showAlert(std::string alertMemo, float duration)
{
    AlertView::getInstance()->_alertType = kAlertTypeAlert;
    
    auto s = cocos2d::Director::getInstance()->getWinSize();
    
    AlertView::getInstance()->_alertMemoLabel->setVisible(true);
    AlertView::getInstance()->_messageBodyBgView->setVisible(false);
    AlertView::getInstance()->_okButton->setVisible(false);
    AlertView::getInstance()->_cancelButton->setVisible(false);
    AlertView::getInstance()->_titleLabel->setVisible(false);
    AlertView::getInstance()->_contentLabel->setVisible(false);
    
    AlertView::getInstance()->_duration = duration;
    
    if (AlertView::getInstance()->_isAlreadyShow) {
        AlertView::getInstance()->setContentSize(cocos2d::Size(s.width, TOP_NOTI_HEIGHT));
        AlertView::getInstance()->setBackgroundColor4F(MAKE_COLOR4F(0xff3a2f, 1.0f));
        AlertView::getInstance()->_alertMemoLabel->setString(alertMemo);
        AlertView::getInstance()->_alertMemoLabel->setPosition(cocos2d::Vec2(s.width/2-AlertView::getInstance()->_alertMemoLabel->getContentSize().width/2, TOP_MENU_HEIGHT/2-AlertView::getInstance()->_alertMemoLabel->getContentSize().height/2));
        
        if (AlertView::getInstance()->isScheduled(schedule_selector(AlertView::setHideAlert))) {
            AlertView::getInstance()->unschedule(schedule_selector(AlertView::setHideAlert));
        }
        _instance->setTag(ALERT_VIEW_ID);
    } else {
        AlertView::getInstance()->setPosition(cocos2d::Vec2(0, s.height));
        AlertView::getInstance()->setContentSize(cocos2d::Size(s.width, TOP_NOTI_HEIGHT));
        AlertView::getInstance()->setBackgroundColor4F(MAKE_COLOR4F(0xff3a2f, 1.0f));
        
        AlertView::getInstance()->_isAlreadyShow = true;
        SMView * layer = (SMView*)cocos2d::Director::getInstance()->getSharedLayer(cocos2d::Director::SharedLayer::POPUP);
        _instance->setTag(ALERT_VIEW_ID);
        layer->addChild(_instance);
        
        AlertView::getInstance()->_alertMemoLabel->setString(alertMemo);
        AlertView::getInstance()->_alertMemoLabel->setPosition(cocos2d::Vec2(s.width/2 - AlertView::getInstance()->_alertMemoLabel->getContentSize().width/2, TOP_MENU_HEIGHT/2-AlertView::getInstance()->_alertMemoLabel->getContentSize().height/2));
        
        auto moveDown = cocos2d::MoveTo::create(ALERT_MOVE_DOWN, cocos2d::Vec2(0, s.height-TOP_MENU_HEIGHT));
        AlertView::getInstance()->runAction(moveDown);
    }
    AlertView::getInstance()->schedule(schedule_selector(AlertView::setHideAlert), AlertView::getInstance()->_duration);
}

void AlertView::setHideAlert(float dt)
{
    if (AlertView::getInstance()->isScheduled(schedule_selector(AlertView::setHideAlert))) {
        AlertView::getInstance()->unschedule(schedule_selector(AlertView::setHideAlert));
    }
    
    auto fadeOut = cocos2d::FadeOut::create(ALERT_FADE_OUT);
    AlertView::getInstance()->runAction(cocos2d::Sequence::create(fadeOut, cocos2d::CallFunc::create([&]{
        AlertView::getInstance()->setOpacity((GLubyte)0xff);
        AlertView::getInstance()->removeFromParent();
        AlertView::getInstance()->_isAlreadyShow = false;
    }), NULL));
}
