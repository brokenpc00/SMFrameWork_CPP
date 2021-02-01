//
//  SMVideoPlayer.h
//  IPCT
//
//  Created by SteveKim on 2019/12/31.
//

#ifndef SMVideoPlayer_h
#define SMVideoPlayer_h

#include <cocos2d.h>
#include "../Base/SMView.h"
#include <string>
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include <cocos/ui/UIVideoPlayer.h>
#endif

class SMButton;
class SMImageView;
class SMVideoPlayer;

class VidoeViewListener
{
public:
    virtual void onDidUpdateStream(SMVideoPlayer * player) = 0;
    
};

enum SMPlayerStatus {
    STOPPED,
    SUSPEND,
    CONNECTING,
    RENDERING,
};

enum SMPlayerType {
    STREAM,
    NORMAL,
    AUDIO_ONLY,
    RECORD,
    RECORD_AUDIO_ONLY,
};

enum SMPlayerRotate {
    VERTICAL,
    HORIZONTAL,
    BOTH,
};

class SMVideoPlayer : public SMView, public OnClickListener//, public AudioCodecProtocol
{
protected:
    SMVideoPlayer();
    virtual ~SMVideoPlayer();
public:
    static SMVideoPlayer * create(const std::string& videoSource="", float x=0, float y=0, float width=0, float height=0, SMPlayerType playerType=SMPlayerType::STREAM, SMPlayerRotate playerRotate=SMPlayerRotate::VERTICAL) {
        SMVideoPlayer * player = new (std::nothrow) SMVideoPlayer();
        player->setAnchorPoint(cocos2d::Vec2::ZERO);
        player->setPosition(cocos2d::Vec2(x, y));
        player->setContentSize(cocos2d::Size(width, height));
        player->_playerType = playerType;
        player->_playerRotate = playerRotate;
        if (player && player->initWithUrl(videoSource)) {
            player->autorelease();
        } else {
            CC_SAFE_DELETE(player);
        }
        
        return player;
    }
    
    #if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
private:
    cocos2d::experimental::ui::VideoPlayer * _videoPlayer;
public:
    void videoEventCallback(cocos2d::Ref* sender, cocos2d::experimental::ui::VideoPlayer::EventType eventType);
    #elif (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
    #else

    #endif

    void play();
    void stop();
    bool isAutoAudio() {return _isAutoAudio;}
    bool isAutoRecord() {return _isAutoRecord;}
    bool showAllRegon() {return _showAllRegon;}
    bool showActiveStatus() {return _showActiveStatus;}
    
    void beginTransform();
    void endTransform();
    
    void hideBtnView();
    void changeHorizontalScreen(bool horizontal);
    
    void startAudioFn();
    void stopAudioFn();
    void startVideoFn();
    void stopVideoFn();
    void flush();
    
protected:

    void pausePlay();
    void resumePlay();
    
    virtual void pause() override;
    virtual void resume() override;
    
    virtual void onClick(SMView * view) override;
    
protected:
    virtual bool initWithUrl(const std::string& videoSource);
    virtual void onEnter() override;
    virtual void onExit() override;
    
    virtual void setContentSize(const cocos2d::Size& contentSize) override;
    
private:
    SMView * _contentView;
    SMImageView * _videoImageView;
    bool _isPlayable;
    bool _visibleControl;
    bool _isAudioPlay;
    bool _isAutoAudio;
    bool _isAutoRecord;
    bool _showAllRegon;
    bool _showActiveStatus;

    SMButton * _audioButton;
    SMButton * _recordButton;
    SMButton * _rotateButton;
    SMButton * _screenshotButton;
    SMButton * _playButton;

    SMPlayerType _playerType;
    SMPlayerStatus _playerStatus;
    SMPlayerRotate _playerRotate;
    
    
};


#endif /* SMVideoPlayer_h */
