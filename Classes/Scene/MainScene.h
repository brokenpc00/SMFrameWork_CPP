//
//  MainScene.h
//  SMCocosTest
//
//  Created by KimSteve on 2017. 4. 13..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#ifndef MainScene_h
#define MainScene_h

#include "../SMFrameWork/Base/SMScene.h"
#include <cocos2d.h>

#define MEDIA_TEST

#ifdef MEDIA_TEST

#include <ui/UIEditBox/UIEditBox.h>

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include <ui/UIVideoPlayer.h>
#endif

#if (CC_TARGET_PLATFORM != CC_PLATFORM_ANDROID)
#include "../SMFrameWork/Util/MosquittoClient.h"
#endif

class VideoSprite;
class SMImageView;

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
class MainScene : public SMScene, public cocos2d::ui::EditBoxDelegate, public cocos2d::IMEDelegate, public MQTTProtocol
{
public:
    MainScene();
    ~MainScene();
    CREATE_SCENE(MainScene);
    
    void videoEventCallback(cocos2d::Ref* sender, cocos2d::experimental::ui::VideoPlayer::EventType eventType);
    
// MQTT protocol
protected:
    virtual void didConnect(int code) override;
    virtual void didDisconnect() override;
    virtual void didPublish(int messageId) override;

    virtual void didReceiveMessage(std::string message, std::string topic) override;
    virtual void didSubscribe(int messageID, std::vector<int> * grantedQos) override;
    virtual void didUnsubscribe(int messageID) override;


protected:
    virtual bool init() override;

    // editbox delegate
    virtual void editBoxEditingDidBegin(cocos2d::ui::EditBox* editBox) override;
    virtual void editBoxEditingDidEnd(cocos2d::ui::EditBox* editBox) override;
    virtual void editBoxTextChanged(cocos2d::ui::EditBox* editBox, const std::string& text) override;
    virtual void editBoxReturn(cocos2d::ui::EditBox* editBox) override;

    // ime delegate
    virtual void keyboardWillShow(cocos2d::IMEKeyboardNotificationInfo& info) override;
    virtual void keyboardWillHide(cocos2d::IMEKeyboardNotificationInfo& info) override;

    void moveUp(float t);
    void moveDown(float t);
    
    void onFrameUpdate(float dt);
    
    void sendText();
private:
    SMView * _contentView;
    VideoSprite * _videoSprite;
    SMImageView * _videoFrame;
    SMView * _playMenu;
    SMView * _container;

    float _keyHeight;
    cocos2d::ui::EditBox * _inputEditBox;
    SMView * _inputEditBoxBG;
    cocos2d::Label * _inputPlaceHolder;
    cocos2d::experimental::ui::VideoPlayer * _videoPlayer;
  
    MosquittoClient * _mqtt;
};
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
class MainScene : public SMScene, public cocos2d::ui::EditBoxDelegate, public cocos2d::IMEDelegate
{
public:
    MainScene();
    ~MainScene();
    CREATE_SCENE(MainScene);
    
    void videoEventCallback(cocos2d::Ref* sender, cocos2d::experimental::ui::VideoPlayer::EventType eventType);

protected:
    virtual bool init() override;

    // editbox delegate
    virtual void editBoxEditingDidBegin(cocos2d::ui::EditBox* editBox) override;
    virtual void editBoxEditingDidEnd(cocos2d::ui::EditBox* editBox) override;
    virtual void editBoxTextChanged(cocos2d::ui::EditBox* editBox, const std::string& text) override;
    virtual void editBoxReturn(cocos2d::ui::EditBox* editBox) override;

    // ime delegate
    virtual void keyboardWillShow(cocos2d::IMEKeyboardNotificationInfo& info) override;
    virtual void keyboardWillHide(cocos2d::IMEKeyboardNotificationInfo& info) override;

    void moveUp(float t);
    void moveDown(float t);
    
    void onFrameUpdate(float dt);
    
    void sendText();
private:
    SMView * _contentView;
    VideoSprite * _videoSprite;
    SMImageView * _videoFrame;
    SMView * _playMenu;
    SMView * _container;

    float _keyHeight;
    cocos2d::ui::EditBox * _inputEditBox;
    SMView * _inputEditBoxBG;
    cocos2d::Label * _inputPlaceHolder;
    cocos2d::experimental::ui::VideoPlayer * _videoPlayer;
  
};
#else
class MainScene : public SMScene, public cocos2d::ui::EditBoxDelegate, public MQTTProtocol
{
public:
    MainScene();
    ~MainScene();
    CREATE_SCENE(MainScene);
    
protected:
    virtual bool init() override;

    // editbox delegate
    virtual void editBoxEditingDidBegin(cocos2d::ui::EditBox* editBox) override;
    virtual void editBoxEditingDidEnd(cocos2d::ui::EditBox* editBox) override;
    virtual void editBoxTextChanged(cocos2d::ui::EditBox* editBox, const std::string& text) override;
    virtual void editBoxReturn(cocos2d::ui::EditBox* editBox) override;

    void onFrameUpdate(float dt);
    void sendText();

// MQTT protocol
protected:
    virtual void didConnect(int code) override;
    virtual void didDisconnect() override;
    virtual void didPublish(int messageId) override;

    virtual void didReceiveMessage(std::string message, std::string topic) override;
    virtual void didSubscribe(int messageID, std::vector<int> * grantedQos) override;
    virtual void didUnsubscribe(int messageID) override;

private:
    SMView * _contentView;
    VideoSprite * _videoSprite;
    SMImageView * _videoFrame;
    SMView * _playMenu;
    SMView * _container;
        
    cocos2d::ui::EditBox * _inputEditBox;
    SMView * _inputEditBoxBG;
    cocos2d::Label * _inputPlaceHolder;
    
    MosquittoClient * _mqtt;
};
#endif

#else
class MainScene : public SMScene
{
public:
    MainScene();
    ~MainScene();
    CREATE_SCENE(MainScene);
    
private:
    virtual bool init() override;
    
    
};
#endif
#endif /* MainScene_h */
