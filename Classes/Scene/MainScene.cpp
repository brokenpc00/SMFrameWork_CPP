//
//  MainScene.cpp
//  MyGame
//
//  Created by KimSteve on 2017. 4. 13..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#include "MainScene.h"
#include "../SMFrameWork/Base/SMButton.h"
#include "../SMFrameWork/Base/SMImageView.h"
#include <cocos2d.h>
//#if (CC_TARGET_PLATFORM != CC_PLATFORM_ANDROID)
#include "../SMFrameWork/Util/SMVideoPlayer.h"
//#endif
#include "../SMFrameWork/Util/ViewUtil.h"
#include "../SMFrameWork/Const/SMFontColor.h"
#include "../SMFrameWork/Base/ShaderNode.h"
#include "../SMFrameWork/Util/encrypt/AES.h"

#define INPK_SERVER
//#define USING_HLS

#ifdef INPK_SERVER
    #ifdef USING_HLS
        #define SERVER_ADDR "http://1.227.62.125/hls/SteveTest.m3u8"
    #else
        #define SERVER_ADDR "rtmp://1.227.62.125/hls/SteveTest"
    #endif
#else
    #ifdef USING_HLS
        #define SERVER_ADDR "http://15.164.50.144/hls/SteveTest.m3u8"
    #else
        #define SERVER_ADDR "rtmp://15.164.50.144/hls/SteveTest"
    #endif
#endif

MainScene::MainScene()
{
    
}

MainScene::~MainScene()
{
    if (isScheduled(schedule_selector(MainScene::onFrameUpdate))) {
        unschedule(schedule_selector(MainScene::onFrameUpdate));
    }
}

bool MainScene::init()
{
    if (!SMScene::init()) {
        return false;
    }
    
    cocos2d::Size s = cocos2d::Director::getInstance()->getWinSize();
    _contentView = SMView::create(0, 0, s.width, s.height);
    addChild(_contentView);

#ifdef MEDIA_TEST

    if (0) {
        #if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
        _videoPlayer = cocos2d::experimental::ui::VideoPlayer::create();
        _videoPlayer->setContentSize(s);
        _videoPlayer->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
        _videoPlayer->setPosition(s/2);
        _videoPlayer->addEventListener(CC_CALLBACK_2(MainScene::videoEventCallback, this));
        _contentView->addChild(_videoPlayer, 1);
        _videoPlayer->setSwallowTouches(false);
        _videoPlayer->setURL(SERVER_ADDR);
        _videoPlayer->setStyle(cocos2d::experimental::ui::VideoPlayer::StyleType::NONE);
        _videoPlayer->play();
        #endif

    } else {
//#if (CC_TARGET_PLATFORM != CC_PLATFORM_ANDROID)
        auto videoPlayer = SMVideoPlayer::create(SERVER_ADDR, 0, 0, s.width, s.height, SMPlayerType::NORMAL);
        _contentView->addChild(videoPlayer);
//#endif
    }
    
    _container = SMView::create(0, 0, s.width, s.height);
    _container->setColor4F(MAKE_COLOR4F(0x000000, 1));
    _contentView->addChild(_container, 2);

//#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    
    float inputBgHeight = 140;
    _inputEditBoxBG = SMView::create(0, 20, 40, s.width-240, inputBgHeight);
    _container->addChild(_inputEditBoxBG);
    
    auto rounded = ShapeRoundedRect::create();
    rounded->setContentSize(cocos2d::Size(_inputEditBoxBG->getContentSize().width, inputBgHeight-20));
    rounded->setPosition(cocos2d::Vec2(20, 10));
    rounded->setLineWidth(ShapeNode::DEFAULT_ANTI_ALIAS_WIDTH*2);
    rounded->setColor4F(MAKE_COLOR4F(0x222222, 1.0f));
    rounded->setCornerRadius(40);
    _inputEditBoxBG->addChild(rounded);
//
    _inputEditBox = cocos2d::ui::EditBox::create(cocos2d::Size(_inputEditBoxBG->getContentSize().width-40, _inputEditBoxBG->getContentSize().height-40), cocos2d::ui::Scale9Sprite::create());
    _inputEditBox->setCascadeColorEnabled(true);
    _inputEditBox->setFontColor(MAKE_COLOR3B(0x222222));
    _inputEditBox->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    _inputEditBox->setPosition(_inputEditBoxBG->getContentSize()/2);
    _inputEditBox->setFontName(SMFontConst::SystemFontRegular);
    _inputEditBox->setFontSize(50);
    _inputEditBox->setReturnType(cocos2d::ui::EditBox::KeyboardReturnType::DONE);
    _inputEditBox->setInputMode(cocos2d::ui::EditBox::InputMode::SINGLE_LINE);
    _inputEditBox->setInputFlag(cocos2d::ui::EditBox::InputFlag::SENSITIVE);
    _inputEditBox->setMaxLength(100);
    _inputEditBox->setDelegate(this);
    _inputEditBox->setTextHorizontalAlignment(cocos2d::TextHAlignment::CENTER);
    _inputEditBoxBG->addChild(_inputEditBox);
    
    _inputEditBox->setFixedGlViewWhenKeyboardOpen(true);
    
    auto sendBtn = SMButton::create(0, SMButton::Style::SOLID_ROUNDEDRECT, s.width-200, 40, 200, inputBgHeight);
    _container->addChild(sendBtn);
    sendBtn->setOutlineWidth(2.0f);
    sendBtn->setOutlineColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x00a1e4, 1.0f));
    sendBtn->setOutlineColor(SMButton::State::PRESSED, cocos2d::Color4F::WHITE);
    sendBtn->setShapeCornerRadius(40);
    sendBtn->setButtonColor(SMButton::State::NORMAL, cocos2d::Color4F::WHITE);
    sendBtn->setButtonColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x00a1e4, 1.0f));
    sendBtn->setTextColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x00a1e4, 1.0f));
    sendBtn->setTextColor(SMButton::State::PRESSED, cocos2d::Color4F::WHITE);
    sendBtn->setTextSystemFont("전송", SMFontConst::SystemFontRegular, 40.0f);

    sendBtn->setOnClickCallback([this](SMView * view){
        sendText();
    });
    
#if (CC_TARGET_PLATFORM != CC_PLATFORM_ANDROID)
    _mqtt = MosquittoClient::create("stevekim", this);
    addChild(_mqtt);
    _mqtt->connectToHost("15.164.50.144");
    _mqtt->subscribe("topic");
#endif
//#endif
#else
    _contentView->setBackgroundColor4F(cocos2d::Color4F(1, 0, 0, 0.3f));
    
    auto programBundle = this->getSceneParam();
    cocos2d::Sprite * originSprite = nullptr;
    if (programBundle) {
        cocos2d::Sprite * tmp = (cocos2d::Sprite *)programBundle->getRef("INTENT_IMAGE");
        if (tmp) {
            originSprite = cocos2d::Sprite::createWithTexture(tmp->getTexture());
        }
    }
    
    auto bgImageView = SMImageView::create();
    bgImageView->setSprite(originSprite);
    bgImageView->setContentSize(s);
    bgImageView->setScaleType(SMImageView::ScaleType::FIT_CENTER);
    _contentView->addChild(bgImageView);
    
//    cocos2d::Label * label = cocos2d::Label::createWithSystemFont("Hello World!!!", "Helvetica", 40.0f);
//    contentView->addChild(label);
//    label->setPosition(contentView->getContentSize()/2);
    
    auto btn = SMButton::create(0, SMButton::Style::DEFAULT, s.width/2-50, s.height/2-40, 100, 80);
    btn->setText("닫기");
    btn->setTextSystemFont("닫기", "Helvetica", 40);
    btn->setTextColor(SMButton::State::NORMAL, cocos2d::Color4F(1, 1, 1, 1));
    btn->setTextColor(SMButton::State::PRESSED, cocos2d::Color4F(1, 0, 1, 1));
    _contentView->addChild(btn);
    
    btn->setOnClickCallback([&](SMView * view){
        cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread([&]{
            
        });
//        cocos2d::Application::getInstance()->stop();
//        cocos2d::Director::getInstance()->end();
//        cocos2d::Director::getInstance()->popScene();
//        cocos2d::Director::getInstance()->
    });
#endif    
    
    
    return true;
}

#ifdef MEDIA_TEST
void MainScene::onFrameUpdate(float dt)
{
    
}

void MainScene::sendText()
{
    std::string msg(_inputEditBox->getText());
    if (msg.length()>0) {
#if (CC_TARGET_PLATFORM != CC_PLATFORM_ANDROID)
        _mqtt->publishString(msg, "topic", false);
#endif
        _inputEditBox->setText("");
    }
}


void MainScene::editBoxEditingDidBegin(cocos2d::ui::EditBox *editBox)
{
//    CCLOG("[[[[[ editBoxEditingDidBegin");
}

void MainScene::editBoxEditingDidEnd(cocos2d::ui::EditBox *editBox)
{
//    CCLOG("[[[[[ editBoxEditingDidEnd");
}

void MainScene::editBoxTextChanged(cocos2d::ui::EditBox *editBox, const std::string &text)
{
//    CCLOG("[[[[[ editBoxTextChanged");
}

void MainScene::editBoxReturn(cocos2d::ui::EditBox *editBox)
{
//    CCLOG("[[[[[ editBoxReturn");
    sendText();
}

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
void MainScene::videoEventCallback(cocos2d::Ref* sender, cocos2d::experimental::ui::VideoPlayer::EventType eventType)
{
    switch (eventType) {
        case cocos2d::experimental::ui::VideoPlayer::EventType::PLAYING:
            CCLOG("PLAYING");
            break;
        case cocos2d::experimental::ui::VideoPlayer::EventType::PAUSED:
            CCLOG("PAUSED");
            break;
        case cocos2d::experimental::ui::VideoPlayer::EventType::STOPPED:
            CCLOG("STOPPED");
            break;
        case cocos2d::experimental::ui::VideoPlayer::EventType::COMPLETED:
            CCLOG("COMPLETED");
            break;
        default:
            break;
    }
}

void MainScene::keyboardWillShow(cocos2d::IMEKeyboardNotificationInfo &info)
{
    _keyHeight = info.begin.size.height;

    if (isScheduled(schedule_selector(MainScene::moveUp))) {
        unschedule(schedule_selector(MainScene::moveUp));
    }
    
    schedule(schedule_selector(MainScene::moveUp));
    
    
//    if (_videoFrame) {
//        auto newSize = cocos2d::Size(_videoFrame->getContentSize().width, _videoFrame->getContentSize().height-_keyHeight);
//        auto resizeTo = cocos2d::ResizeTo::create(0.15f, newSize);
//        auto newPos = cocos2d::Vec2(0, _keyHeight);
//        auto moveTo = cocos2d::MoveTo::create(0.15f, newPos);
//        _videoFrame->runAction(resizeTo);
//        _videoFrame->runAction(moveTo);
//    }
    
    
//    CCLOG("[[[[[ keyboardWillShow");
}

void MainScene::moveUp(float t)
{
    if (isScheduled(schedule_selector(MainScene::moveUp))) {
        unschedule(schedule_selector(MainScene::moveUp));
    }

    auto moveTo = cocos2d::MoveTo::create(0.25f, cocos2d::Vec2(0, _keyHeight));
    _container->runAction(moveTo);
}

void MainScene::keyboardWillHide(cocos2d::IMEKeyboardNotificationInfo &info)
{
//    CCLOG("[[[[[ keyboardWillHide");

    if (isScheduled(schedule_selector(MainScene::moveDown))) {
        unschedule(schedule_selector(MainScene::moveDown));
    }
    
    schedule(schedule_selector(MainScene::moveDown));
}

void MainScene::moveDown(float t)
{
    if (isScheduled(schedule_selector(MainScene::moveDown))) {
        unschedule(schedule_selector(MainScene::moveDown));
    }

    auto moveTo = cocos2d::MoveTo::create(0.25f, cocos2d::Vec2(0, 0));
    _container->runAction(cocos2d::Sequence::create(moveTo, cocos2d::CallFunc::create([&]{
        
    }), NULL));
    
}


#endif

#if (CC_TARGET_PLATFORM != CC_PLATFORM_ANDROID)
void MainScene::didConnect(int code)
{
    CCLOG("[[[[[ %s", __FUNCTION__);
}

void MainScene::didDisconnect()
{
    CCLOG("[[[[[ %s", __FUNCTION__);
}

void MainScene::didPublish(int messageId)
{
    CCLOG("[[[[[ %s", __FUNCTION__);
}

void MainScene::didReceiveMessage(std::string message, std::string topic)
{
    CCLOG("[[[[[ %s : %s -> %s", __FUNCTION__, topic.c_str(), message.c_str());
}

void MainScene::didSubscribe(int messageID, std::vector<int> * grantedQos)
{
    CCLOG("[[[[[ %s : %d", __FUNCTION__, messageID);
}

void MainScene::didUnsubscribe(int messageID)
{
    CCLOG("[[[[[ %s", __FUNCTION__);
}
#endif

#endif
